#include "Order.hpp"

const char* to_string(Side side) {
    return side == Side::Buy ? "BUY" : "SELL";
}
