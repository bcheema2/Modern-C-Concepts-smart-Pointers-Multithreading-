//
// Created by Bibenpreet Cheema on 9/19/26.
//

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <numeric>

// Include your header
#include "include/buffer.hpp"

using namespace RingBuffer;

struct BenchmarkResult {
    double total_time_sec;
    double ops_per_sec;
    double avg_latency_ns;
    std::size_t total_operations;
};

// ---------------------------------------------------------------------------
// Core Benchmark Engine
// ---------------------------------------------------------------------------
BenchmarkResult run_benchmark(std::size_t num_producers,
                              std::size_t num_consumers,
                              std::size_t items_per_producer,
                              std::size_t capacity)
{
    const std::size_t total_items = num_producers * items_per_producer;
    ThreadSafeRingBuffer<std::size_t> buffer(capacity);

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    // Ready latch to ensure all threads start concurrently
    std::atomic<bool> start_flag{false};

    // Spawn Producers
    for (std::size_t p = 0; p < num_producers; ++p) {
        producers.emplace_back([&, p]() {
            while (!start_flag.load(std::memory_order_relaxed)) {
                std::this_thread::yield();
            }
            for (std::size_t i = 0; i < items_per_producer; ++i) {
                buffer.push(i);
            }
        });
    }

    // Spawn Consumers
    for (std::size_t c = 0; c < num_consumers; ++c) {
        consumers.emplace_back([&]() {
            while (!start_flag.load(std::memory_order_relaxed)) {
                std::this_thread::yield();
            }
            std::size_t item = 0;
            while (buffer.pop(item)) {
                // Keep consumer active
                (void)item;
            }
        });
    }

    // Measure Execution Time
    auto start_time = std::chrono::high_resolution_clock::now();

    // Unblock threads simultaneously
    start_flag.store(true, std::memory_order_relaxed);

    // Wait for all producers to finish
    for (auto& t : producers) {
        t.join();
    }

    // Signal consumers to drain remaining items and exit
    buffer.close();

    // Wait for all consumers to finish
    for (auto& t : consumers) {
        t.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end_time - start_time;

    // Total Operations = Total Pushes + Total Pops
    std::size_t total_ops = total_items * 2;

    BenchmarkResult result;
    result.total_time_sec = elapsed.count();
    result.ops_per_sec = static_cast<double>(total_ops) / result.total_time_sec;
    result.avg_latency_ns = (result.total_time_sec / static_cast<double>(total_ops)) * 1e9;
    result.total_operations = total_ops;

    return result;
}

// Helper to format output neatly
void print_row(const std::string& name, std::size_t prod, std::size_t cons, std::size_t cap, const BenchmarkResult& res) {
    std::cout << std::left << std::setw(20) << name
              << std::setw(8) << (std::to_string(prod) + ":" + std::to_string(cons))
              << std::setw(10) << cap
              << std::setw(14) << std::fixed << std::setprecision(3) << res.total_time_sec
              << std::setw(18) << static_cast<std::size_t>(res.ops_per_sec)
              << std::setw(14) << std::fixed << std::setprecision(1) << res.avg_latency_ns
              << "\n";
}

int main() {
    std::cout << "=================================================================================\n";
    std::cout << "                ThreadSafeRingBuffer Performance Benchmark                       \n";
    std::cout << "=================================================================================\n";
    std::cout << std::left << std::setw(20) << "Workload"
              << std::setw(8) << "P:C"
              << std::setw(10) << "Capacity"
              << std::setw(14) << "Time (sec)"
              << std::setw(18) << "Ops/sec"
              << std::setw(14) << "Avg Latency (ns)"
              << "\n";
    std::cout << "---------------------------------------------------------------------------------\n";

    constexpr std::size_t ITEMS_PER_PRODUCER = 1'000'000;

    // Scenario 1: Single Producer, Single Consumer (1:1) - Small Buffer
    auto res1 = run_benchmark(1, 1, ITEMS_PER_PRODUCER, 64);
    print_row("Single-Pair Small", 1, 1, 64, res1);

    // Scenario 2: Single Producer, Single Consumer (1:1) - Large Buffer
    auto res2 = run_benchmark(1, 1, ITEMS_PER_PRODUCER, 4096);
    print_row("Single-Pair Large", 1, 1, 4096, res2);

    // Scenario 3: Symmetric Multi-threaded (4:4)
    auto res3 = run_benchmark(4, 4, ITEMS_PER_PRODUCER, 1024);
    print_row("Symmetric Heavy", 4, 4, 1024, res3);

    // Scenario 4: High Producer Contention (8 Producers : 2 Consumers)
    auto res4 = run_benchmark(8, 2, ITEMS_PER_PRODUCER / 2, 512);
    print_row("Producer Heavy", 8, 2, 512, res4);

    // Scenario 5: High Consumer Contention (2 Producers : 8 Consumers)
    auto res5 = run_benchmark(2, 8, ITEMS_PER_PRODUCER * 2, 512);
    print_row("Consumer Heavy", 2, 8, 512, res5);

    std::cout << "=================================================================================\n";

    return 0;
}
