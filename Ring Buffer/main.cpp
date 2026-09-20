#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
#include <atomic>
#include <chrono>
#include <cassert>

// Include your header here
#include "include/buffer.hpp"
#include "include/concepts.hpp"

using namespace RingBuffer;

// ---------------------------------------------------------------------------
// TEST 1: Stress Test (Multiple Producers & Consumers)
// Verifies that no items are lost, duplicated, or corrupted under high load.
// ---------------------------------------------------------------------------
void test_producer_consumer_stress() {
    std::cout << "[RUNNING] Test 1: Producer-Consumer Stress Test..." << std::flush;

    constexpr std::size_t BUFFER_CAPACITY = 10;
    constexpr std::size_t NUM_PRODUCERS = 4;
    constexpr std::size_t NUM_CONSUMERS = 4;
    constexpr std::size_t ITEMS_PER_PRODUCER = 25'000;
    constexpr std::size_t TOTAL_ITEMS = NUM_PRODUCERS * ITEMS_PER_PRODUCER;

    ThreadSafeRingBuffer<int> buffer(BUFFER_CAPACITY);
    std::atomic<std::size_t> total_consumed_count{0};
    std::atomic<long long> total_sum_consumed{0};

    // Calculate expected sum of all produced numbers
    long long expected_sum = 0;
    for (std::size_t p = 0; p < NUM_PRODUCERS; ++p) {
        for (std::size_t i = 1; i <= ITEMS_PER_PRODUCER; ++i) {
            expected_sum += static_cast<long long>(i);
        }
    }

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    // Launch Producer Threads
    for (std::size_t p = 0; p < NUM_PRODUCERS; ++p) {
        producers.emplace_back([&buffer]() {
            for (std::size_t i = 1; i <= ITEMS_PER_PRODUCER; ++i) {
                buffer.push(static_cast<int>(i));
            }
        });
    }

    // Launch Consumer Threads
    for (std::size_t c = 0; c < NUM_CONSUMERS; ++c) {
        consumers.emplace_back([&buffer, &total_consumed_count, &total_sum_consumed]() {
            int val = 0;
            while (buffer.pop(val)) {
                total_consumed_count.fetch_add(1, std::memory_order_relaxed);
                total_sum_consumed.fetch_add(val, std::memory_order_relaxed);
            }
        });
    }

    // Wait for all producers to finish pushing
    for (auto& t : producers) {
        t.join();
    }

    // Close buffer to signal consumers that no more items are coming
    buffer.close();

    // Wait for all consumers to finish draining and exit
    for (auto& t : consumers) {
        t.join();
    }

    // Assertions
    assert(total_consumed_count.load() == TOTAL_ITEMS && "Item count mismatch!");
    assert(total_sum_consumed.load() == expected_sum && "Data corruption detected! Sum mismatch.");

    std::cout << " [PASSED]\n";
    std::cout << "  -> Total Processed: " << total_consumed_count.load() << " items cleanly.\n";
}

// ---------------------------------------------------------------------------
// TEST 2: Shutdown Unblocking Test
// Verifies that blocked consumer threads wake up immediately on close().
// ---------------------------------------------------------------------------
void test_shutdown_unblocking() {
    std::cout << "[RUNNING] Test 2: Shutdown Unblocking Test..." << std::flush;

    constexpr std::size_t BUFFER_CAPACITY = 5;
    ThreadSafeRingBuffer<int> buffer(BUFFER_CAPACITY);

    std::atomic<bool> consumer_woke_up{false};
    std::atomic<bool> consumer_exited_cleanly{false};

    // Launch a consumer thread on an empty buffer
    std::thread consumer([&]() {
        int item = 0;
        consumer_woke_up = true;

        // This will block because the buffer is empty
        bool popped = buffer.pop(item);

        // If unblocked by close(), pop() should return false
        if (!popped) {
            consumer_exited_cleanly = true;
        }
    });

    // Wait to ensure consumer is actively sleeping in pop()
    while (!consumer_woke_up) {
        std::this_thread::yield();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Close the buffer while consumer is blocked
    buffer.close();

    // Join thread (if close() didn't wake it, join() would hang forever)
    consumer.join();

    assert(consumer_exited_cleanly && "Consumer failed to exit cleanly on close()!");
    std::cout << " [PASSED]\n";
    std::cout << "  -> Blocked thread unblocked and exited gracefully.\n";
}

// ---------------------------------------------------------------------------
// TEST 3: Producer Shutdown Exception Test
// Verifies that push/emplace throw std::runtime_error when closed.
// ---------------------------------------------------------------------------
void test_producer_closed_exception() {
    std::cout << "[RUNNING] Test 3: Closed Buffer Exception Test..." << std::flush;

    ThreadSafeRingBuffer<int> buffer(2);
    buffer.close();

    bool push_threw = false;
    try {
        buffer.push(10);
    } catch (const std::runtime_error&) {
        push_threw = true;
    }

    assert(push_threw && "push() failed to throw exception on closed buffer!");
    std::cout << " [PASSED]\n";
}

int main() {
    std::cout << "===========================================\n";
    std::cout << " Running ThreadSafeRingBuffer Test Suite   \n";
    std::cout << "===========================================\n\n";

    test_producer_consumer_stress();
    test_shutdown_unblocking();
    test_producer_closed_exception();

    std::cout << "\nAll unit tests passed successfully!\n";
    return 0;
}