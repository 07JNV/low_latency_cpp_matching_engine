#include "OrderGenerator.hpp"

#include <random>

OrderGenerator::OrderGenerator(std::uint64_t seed) : seed_(seed) {}

std::vector<Order> OrderGenerator::generate(std::size_t count) const {
    std::mt19937_64 rng(seed_);
    std::uniform_int_distribution<int> side_distribution(0, 1);
    std::uniform_int_distribution<int> price_distribution(9950, 10050);
    std::uniform_int_distribution<int> quantity_distribution(1, 10);

    std::vector<Order> orders;
    orders.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        orders.push_back(Order{
            static_cast<std::uint64_t>(i + 1),
            side_distribution(rng) == 0 ? Side::Buy : Side::Sell,
            OrderType::Limit,
            price_distribution(rng),
            static_cast<std::uint32_t>(quantity_distribution(rng))
        });
    }
    return orders;
}
