#include "MatchingEngine.hpp"
#include "OrderGenerator.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace {
using Clock = std::chrono::steady_clock;

std::uint64_t now_ns() {
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            Clock::now().time_since_epoch()).count());
}

double percentile(std::vector<std::uint64_t> values, double p) {
    if (values.empty()) return 0.0;
    std::sort(values.begin(), values.end());
    const auto index = static_cast<std::size_t>(
        p * static_cast<double>(values.size() - 1));
    return static_cast<double>(values[index]);
}

std::size_t parse_order_count(int argc, char** argv) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == "--orders") {
            return static_cast<std::size_t>(std::stoull(argv[i + 1]));
        }
    }
    return 100000;
}
}

int main(int argc, char** argv) {
    const std::size_t order_count = parse_order_count(argc, argv);
    if (order_count == 0) {
        std::cerr << "Order count must be greater than zero.\n";
        return 1;
    }

    OrderGenerator generator;
    const auto orders = generator.generate(order_count);

    SpscRingBuffer<Command, 65536> queue;
    MatchingEngine engine(queue);
    std::atomic<bool> producer_done{false};

    const auto start = Clock::now();

    std::thread consumer([&] {
        while (!producer_done.load(std::memory_order_acquire) ||
               engine.processed_commands() < orders.size()) {
            if (!engine.process_one()) std::this_thread::yield();
        }
    });

    std::thread producer([&] {
        for (const auto& order : orders) {
            Command command{order, now_ns()};
            while (!queue.try_push(command)) {
                std::this_thread::yield();
            }
        }
        producer_done.store(true, std::memory_order_release);
    });

    producer.join();
    consumer.join();
    const auto end = Clock::now();

    const auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        end - start).count();
    const double seconds = static_cast<double>(elapsed_ns) / 1'000'000'000.0;
    const double throughput = seconds > 0.0
        ? static_cast<double>(engine.processed_commands()) / seconds : 0.0;

    // This queue-wait estimate is producer-side push-loop duration, not engine
    // processing latency. The output labels it honestly.
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Low-Latency C++ Matching Engine\n"
              << "===============================\n"
              << "Generated commands: " << orders.size() << '\n'
              << "Processed commands: " << engine.processed_commands() << '\n'
              << "Trades:             " << engine.order_book().trades().size() << '\n'
              << "Resting orders:     " << engine.order_book().resting_order_count() << '\n'
              << "Elapsed:            " << seconds << " s\n"
              << "Throughput:         " << throughput << " commands/s\n"
              << "Enqueue-to-engine p50: " << percentile(engine.latencies_ns(), 0.50) << " ns\n"
              << "Enqueue-to-engine p99: " << percentile(engine.latencies_ns(), 0.99) << " ns\n"
              << "\nNote: these are local simulator measurements, not exchange latency.\n";

    return 0;
}
