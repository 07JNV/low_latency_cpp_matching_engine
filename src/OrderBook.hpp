#pragma once

#include "Order.hpp"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <unordered_map>
#include <vector>

class OrderBook {
public:
    bool add_limit_order(const Order& order);
    bool cancel_order(std::uint64_t order_id);
    std::size_t resting_order_count() const noexcept;
    const std::vector<Trade>& trades() const noexcept;

private:
    struct RestingOrder {
        std::uint64_t id;
        std::uint32_t remaining_quantity;
    };

    // bids: highest price first; asks: lowest price first.
    std::map<std::int64_t, std::deque<RestingOrder>, std::greater<std::int64_t>> bids_;
    std::map<std::int64_t, std::deque<RestingOrder>, std::less<std::int64_t>> asks_;

    // Maps order ID to side, price, and its position in a price-level queue.
    // A deque iterator is stable for pushes at either end, but erasing can invalidate
    // iterators depending on implementation; therefore cancellation scans its level.
    struct OrderLocation { Side side; std::int64_t price; };
    std::unordered_map<std::uint64_t, OrderLocation> locations_;

    std::vector<Trade> trades_;

    void match_buy(Order incoming);
    void match_sell(Order incoming);
    void rest_order(const Order& order, std::uint32_t remaining_quantity);
    void remove_from_index_if_empty(Side side, std::int64_t price);
};
