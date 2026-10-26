#pragma once

#include <cstdint>
#include <string>

enum class Side : std::uint8_t { Buy, Sell };
enum class OrderType : std::uint8_t { Limit, Cancel };

struct Order {
    std::uint64_t id{0};
    Side side{Side::Buy};
    OrderType type{OrderType::Limit};
    std::int64_t price{0};       // integer ticks; positive for limit orders
    std::uint32_t quantity{0};   // positive for new limit orders
};

struct Command {
    Order order{};
    std::uint64_t enqueue_timestamp_ns{0};
};

struct Trade {
    std::uint64_t buy_order_id{0};
    std::uint64_t sell_order_id{0};
    std::int64_t price{0};
    std::uint32_t quantity{0};
};

const char* to_string(Side side);
