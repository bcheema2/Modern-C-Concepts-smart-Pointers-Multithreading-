#Ring Buffer
Created this project from ground up for practicing concepts, multithreading and concurrency in modern C++ 20. Adding benchmarks results; I researched a bit and can make these results more better but will come back later. I need to focus on more small projects to get a good grasp on concurrency and concepts.
##Performance Benchmark
Evaluated on Apple Silicon(M5 Air) (ARM64) using `std::chrono::high_resolution_clock` across 1,000,000 operations per producer.

| Workload | Producer:Consumer Ratio | Buffer Capacity | Execution Time (s) | Throughput (Ops/sec) | Avg Latency (ns) |
| :--- | :---: | :---: | :---: | :---: | :---: |
| **Single-Pair Small** | 1:1 | 64 | 0.074 | 26.8 M | 37.2 |
| **Single-Pair Large** | 1:1 | 4096 | 0.058 | 34.6 M | 28.9 |
| **Symmetric Heavy** | 4:4 | 1024 | 0.475 | 16.8 M | 59.4 |
| **Producer Heavy** | 8:2 | 512 | 1.689 | 4.7 M | 211.1 |
| **Consumer Heavy** | 2:8 | 512 | 1.961 | 4.0 M | 245.2 |

### Performance Key Takeaways
* **Peak Throughput:** Reaches **~34.6 Million ops/sec** (~28.9 ns latency) in single-producer single-consumer (SPSC) mode with a large buffer capacity (4096).
* **Capacity Impact:** Increasing buffer capacity from 64 to 4096 improves throughput by **~28%** by minimizing condition variable context switches.
* **Contention Scaling:** Under high multi-threaded contention (8:2 / 2:8), throughput scales down smoothly to **~4.0M ops/sec** due to `std::mutex` lock acquisition overhead and OS thread scheduling.
