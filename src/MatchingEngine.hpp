#pragma once

#include "OrderBook.hpp"
#include "SpscRingBuffer.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

class MatchingEngine {
public:
    explicit MatchingEngine(SpscRingBuffer<Command, 65536>& queue);

    // Called only on the consumer/engine thread.
    bool process_one();
    void run(const std::atomic<bool>& producer_done);

    std::uint64_t processed_commands() const noexcept;
    const OrderBook& order_book() const noexcept;
    const std::vector<std::uint64_t>& latencies_ns() const noexcept;

private:
    SpscRingBuffer<Command, 65536>& queue_;
    OrderBook order_book_;
    std::uint64_t processed_commands_{0};
    std::vector<std::uint64_t> latencies_ns_; // written/read only across thread join
};
