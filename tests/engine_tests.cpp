#include "OrderBook.hpp"

#include <cstdlib>
#include <iostream>

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "TEST FAILURE: " << message << '\n';
        std::exit(1);
    }
}

int main() {
    OrderBook book;

    // A resting sell at 100, then a buy at 100 should execute at resting price 100.
    require(book.add_limit_order(Order{1, Side::Sell, OrderType::Limit, 100, 5}),
            "accept initial sell");
    require(book.add_limit_order(Order{2, Side::Buy, OrderType::Limit, 100, 3}),
            "accept crossing buy");
    require(book.trades().size() == 1, "one trade after partial fill");
    require(book.trades()[0].price == 100, "trade executes at resting price");
    require(book.trades()[0].quantity == 3, "trade quantity is min of order quantities");
    require(book.resting_order_count() == 1, "sell remainder rests in book");

    // Buy 3 more at 100 completes the resting sell and leaves 1 buy resting.
    require(book.add_limit_order(Order{3, Side::Buy, OrderType::Limit, 100, 3}),
            "accept second crossing buy");
    require(book.trades().size() == 2, "second trade recorded");
    require(book.trades()[1].quantity == 2, "only remaining sell quantity executes");
    require(book.resting_order_count() == 1, "incoming buy remainder rests");

    // Cancellation removes an unfilled resting order.
    require(book.cancel_order(3), "cancel resting buy");
    require(!book.cancel_order(3), "cannot cancel the same order twice");
    require(book.resting_order_count() == 0, "book empty after cancellation");

    // Invalid order should be rejected.
    require(!book.add_limit_order(Order{4, Side::Buy, OrderType::Limit, 0, 10}),
            "reject non-positive price");

    std::cout << "All matching-engine tests passed.\n";
    return 0;
}
