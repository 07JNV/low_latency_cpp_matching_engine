#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <type_traits>

// Bounded queue for exactly one producer thread and one consumer thread.
// Capacity must be a power of two. One slot is kept empty to distinguish full from empty.
template <typename T, std::size_t Capacity>
class SpscRingBuffer {
    static_assert(Capacity >= 2, "Capacity must be at least 2");
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of two");
    static_assert(std::is_trivially_copyable<T>::value,
                  "This educational queue expects trivially copyable values");

public:
    bool try_push(const T& value) noexcept {
        const auto head = head_.load(std::memory_order_relaxed);
        const auto next = (head + 1) & mask_;
        if (next == tail_.load(std::memory_order_acquire)) {
            return false; // full
        }

        buffer_[head] = value;
        head_.store(next, std::memory_order_release); // publish item
        return true;
    }

    bool try_pop(T& value) noexcept {
        const auto tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_.load(std::memory_order_acquire)) {
            return false; // empty
        }

        value = buffer_[tail];
        tail_.store((tail + 1) & mask_, std::memory_order_release);
        return true;
    }

    static constexpr std::size_t usable_capacity() noexcept { return Capacity - 1; }

private:
    static constexpr std::size_t mask_ = Capacity - 1;

    // Separate cache lines to reduce false sharing between producer and consumer.
    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_{0};
    std::array<T, Capacity> buffer_{};
};
