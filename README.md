# Low-Latency C++ Matching Engine

A from-scratch, class-based educational limit-order-book and matching-engine project.

## Scope

This project is a single-process simulator, not a production exchange. It demonstrates:
- Limit orders and price-time priority
- Order book management and cancellation
- A bounded single-producer/single-consumer (SPSC) ring buffer using C++ atomics
- Separate producer and matching-engine threads
- Latency and throughput measurement
- Basic tests

The matching engine is single-threaded by design: only its engine thread mutates the order book. This avoids locks inside the matching logic. The producer communicates with it through the SPSC queue.

## Requirements

- C++17 compiler (GCC 9+ or Clang 10+ recommended)
- CMake 3.16+
- Linux/macOS; Linux is recommended for `perf`

## Build and run

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/matching_engine --orders 100000
```

Run tests:

```bash
ctest --test-dir build --output-on-failure
```

Optional benchmark script (uses the executable):

```bash
python3 scripts/benchmark.py
```

## Example

The program generates synthetic orders, submits them from a producer thread, and processes them on the engine thread. It reports processed commands/sec, execution count, and enqueue-to-engine-processing latency percentiles.

Latency is measured inside this local simulator; it is not a claim about exchange/network latency.

## Design overview

```text
OrderGenerator (producer thread)
        |
        v
SpscRingBuffer<Command, 65536>
        |
        v
MatchingEngine (engine thread)
        |
        +--> OrderBook
                +--> bids: price descending, FIFO within each price
                +--> asks: price ascending, FIFO within each price
```

Commands are passed by value. `SpscRingBuffer` is only correct with exactly one producer and one consumer. Do not add producers without changing the queue design.

## Important implementation notes

- Prices are integer ticks, not floating-point numbers. This avoids floating-point comparison errors.
- Quantities are positive integers.
- Each price level is FIFO, so older orders at the same price execute first.
- Incoming buy orders cross when their limit price is greater than or equal to the best ask.
- Incoming sell orders cross when their limit price is less than or equal to the best bid.
- A partially filled resting order stays in the book with its remaining quantity.
- A partially filled incoming order rests in the book with its remaining quantity.
- Cancellation removes an order from its price level and index.
- The SPSC queue uses acquire/release ordering to publish items safely between threads.
- This is intentionally a learning project. A production engine needs extensive validation, risk controls, recovery, sequencing, and more rigorous performance engineering.
