#pragma once

#include "Order.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

class OrderGenerator {
public:
    explicit OrderGenerator(std::uint64_t seed = 42);
    std::vector<Order> generate(std::size_t count) const;

private:
    std::uint64_t seed_;
};
