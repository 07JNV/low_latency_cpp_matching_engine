#include "OrderBook.hpp"

#include <algorithm>

bool OrderBook::add_limit_order(const Order& order) {
    if (order.type != OrderType::Limit || order.id == 0 ||
        order.price <= 0 || order.quantity == 0 ||
        locations_.find(order.id) != locations_.end()) {
        return false;
    }

    if (order.side == Side::Buy) {
        match_buy(order);
    } else {
        match_sell(order);
    }
    return true;
}

void OrderBook::match_buy(Order incoming) {
    auto remaining = incoming.quantity;

    while (remaining > 0 && !asks_.empty()) {
        auto best = asks_.begin();
        if (best->first > incoming.price) break;

        auto& queue = best->second;
        while (remaining > 0 && !queue.empty()) {
            auto& resting = queue.front();
            const auto executed = std::min(remaining, resting.remaining_quantity);

            trades_.push_back(Trade{incoming.id, resting.id, best->first, executed});
            remaining -= executed;
            resting.remaining_quantity -= executed;

            if (resting.remaining_quantity == 0) {
                locations_.erase(resting.id);
                queue.pop_front();
            }
        }

        if (queue.empty()) asks_.erase(best);
    }

    if (remaining > 0) rest_order(incoming, remaining);
}

void OrderBook::match_sell(Order incoming) {
    auto remaining = incoming.quantity;

    while (remaining > 0 && !bids_.empty()) {
        auto best = bids_.begin();
        if (best->first < incoming.price) break;

        auto& queue = best->second;
        while (remaining > 0 && !queue.empty()) {
            auto& resting = queue.front();
            const auto executed = std::min(remaining, resting.remaining_quantity);

            trades_.push_back(Trade{resting.id, incoming.id, best->first, executed});
            remaining -= executed;
            resting.remaining_quantity -= executed;

            if (resting.remaining_quantity == 0) {
                locations_.erase(resting.id);
                queue.pop_front();
            }
        }

        if (queue.empty()) bids_.erase(best);
    }

    if (remaining > 0) rest_order(incoming, remaining);
}

void OrderBook::rest_order(const Order& order, std::uint32_t remaining_quantity) {
    if (order.side == Side::Buy) {
        bids_[order.price].push_back({order.id, remaining_quantity});
    } else {
        asks_[order.price].push_back({order.id, remaining_quantity});
    }
    locations_[order.id] = {order.side, order.price};
}

bool OrderBook::cancel_order(std::uint64_t order_id) {
    const auto location = locations_.find(order_id);
    if (location == locations_.end()) return false;

    const auto side = location->second.side;
    const auto price = location->second.price;

    if (side == Side::Buy) {
        auto level = bids_.find(price);
        if (level == bids_.end()) return false;
        auto& queue = level->second;
        const auto it = std::find_if(queue.begin(), queue.end(),
            [order_id](const RestingOrder& order) { return order.id == order_id; });
        if (it == queue.end()) return false;
        queue.erase(it);
        if (queue.empty()) bids_.erase(level);
    } else {
        auto level = asks_.find(price);
        if (level == asks_.end()) return false;
        auto& queue = level->second;
        const auto it = std::find_if(queue.begin(), queue.end(),
            [order_id](const RestingOrder& order) { return order.id == order_id; });
        if (it == queue.end()) return false;
        queue.erase(it);
        if (queue.empty()) asks_.erase(level);
    }

    locations_.erase(location);
    return true;
}

std::size_t OrderBook::resting_order_count() const noexcept {
    return locations_.size();
}

const std::vector<Trade>& OrderBook::trades() const noexcept {
    return trades_;
}
