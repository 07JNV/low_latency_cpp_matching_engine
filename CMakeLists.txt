cmake_minimum_required(VERSION 3.16)
project(low_latency_matching_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_executable(matching_engine
    src/main.cpp
    src/Order.cpp
    src/OrderBook.cpp
    src/MatchingEngine.cpp
    src/OrderGenerator.cpp
)
target_include_directories(matching_engine PRIVATE src)
target_link_libraries(matching_engine PRIVATE Threads::Threads)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(matching_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_executable(engine_tests
    tests/engine_tests.cpp
    src/Order.cpp
    src/OrderBook.cpp
    src/MatchingEngine.cpp
)
target_include_directories(engine_tests PRIVATE src)
target_link_libraries(engine_tests PRIVATE Threads::Threads)
add_test(NAME engine_tests COMMAND engine_tests)
