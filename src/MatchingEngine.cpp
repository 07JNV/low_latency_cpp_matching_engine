#include "MatchingEngine.hpp"

#include <chrono>
#include <thread>

namespace {
std::uint64_t steady_now_ns() {
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
}
}

MatchingEngine::MatchingEngine(SpscRingBuffer<Command, 65536>& queue)
    : queue_(queue) {}

bool MatchingEngine::process_one() {
    Command command;
    if (!queue_.try_pop(command)) return false;

    const auto processed_at_ns = steady_now_ns();
    latencies_ns_.push_back(processed_at_ns - command.enqueue_timestamp_ns);

    if (command.order.type == OrderType::Cancel) {
        order_book_.cancel_order(command.order.id);
    } else {
        order_book_.add_limit_order(command.order);
    }
    ++processed_commands_;
    return true;
}

void MatchingEngine::run(const std::atomic<bool>& producer_done) {
    // Simple polling loop for clarity. Production systems may use adaptive spinning,
    // pause instructions, or a blocking mechanism depending on latency/power goals.
    while (!producer_done.load(std::memory_order_acquire) ||
           processed_commands_ == 0) {
        if (!process_one()) {
            if (producer_done.load(std::memory_order_acquire)) {
                // Drain anything that was published just before producer_done.
                if (!process_one()) break;
            } else {
                std::this_thread::yield();
            }
        }
    }

    // Final drain in case the producer finished immediately after publishing.
    while (process_one()) {}
}

std::uint64_t MatchingEngine::processed_commands() const noexcept {
    return processed_commands_;
}

const OrderBook& MatchingEngine::order_book() const noexcept {
    return order_book_;
}

const std::vector<std::uint64_t>& MatchingEngine::latencies_ns() const noexcept {
    return latencies_ns_;
}
