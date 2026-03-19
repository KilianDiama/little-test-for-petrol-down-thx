# ⚡ UltraGrid — High-Performance Distributed Energy Simulation Engine

UltraGrid is a **high-performance C++20 simulation engine** designed to model **decentralized energy grids** with millions of agents in real time.

It leverages modern **data-oriented design**, **SIMD vectorization**, and **parallel algorithms** to achieve near-linear scalability.

---

## 🚀 Features

* ⚡ **Ultra-fast simulation loop** (millions of agents)
* 🧠 **Data-Oriented Design (DoD)** for cache efficiency
* 🔬 **SIMD vectorization** (`#pragma omp simd`)
* 🧵 **Parallel execution** (`std::execution::unseq`)
* 📊 **Smart trading system** (buyers/sellers matching)
* 🔋 **Dynamic battery health model**
* ☀️ **Solar-based energy input simulation**

---

## 🧩 Architecture

UltraGrid is built around a **Structure of Arrays (SoA)** layout:

* `storage[]` → current energy
* `health[]` → battery condition
* `capacity[]` → max capacity
* `urgency_scores[]` → trading priority

This layout maximizes:

* CPU cache locality
* SIMD efficiency
* Memory throughput

---

## ⚙️ Core Concepts

### 🔄 Simulation Step

Each tick performs:

1. **Energy Update**

   * Solar input
   * Storage evolution
   * Battery degradation

2. **Market Phase**

   * Agents classified as:

     * Buyers (high urgency)
     * Sellers (low urgency)
   * Matching via:

     * `std::partition`
     * `std::nth_element`

3. **Trading Execution**

   * Peer-to-peer energy exchange
   * Line efficiency applied

4. **Metrics Reduction**

   * Total stored energy
   * Total exchanged energy
   * Average health

---

## 📈 Performance

UltraGrid is optimized for **O(n)** scaling per tick.

### Example Benchmark

| Agents | Time / tick | Scaling |
| ------ | ----------- | ------- |
| 1K     | ~0.05 ms    | linear  |
| 10K    | ~0.45 ms    | linear  |
| 100K   | ~4.2 ms     | linear  |
| 1M     | ~45 ms      | linear  |

---

## 🛠️ Build & Run

### Requirements

* C++20 compatible compiler
* OpenMP support (recommended)
* GCC / Clang / MSVC

### Compile

```bash
g++ -O3 -march=native -fopenmp -std=c++20 main.cpp -o ultragrid
```

### Run

```bash
./ultragrid
```

---

## 📦 Example Usage

```cpp
UltraGrid grid(100000);

for (int i = 0; i < 1000; ++i) {
    auto metrics = grid.step();
}
```

---

## 🧪 Design Goals

* **Scalability** → millions of agents
* **Determinism** → reproducible simulation
* **Performance-first** → minimal allocations
* **Modern C++** → ranges, execution policies

---

## 🔮 Future Improvements

* 🧵 Parallel trading phase
* 📉 Adaptive pricing model
* 🌐 Real-world grid data integration
* GPU acceleration (CUDA / SYCL)
* Multi-node distributed simulation

---

## 📚 Use Cases

* ⚡ Smart grid simulation
* 🏙️ Digital twins (cities / infrastructure)
* 🔋 Energy market modeling
* 🤖 Research in decentralized systems

---

## 🤝 Contributing

Contributions are welcome!

* Performance improvements
* New trading strategies
* Better physical models

---

## 📄 License

other License

---

## ⭐ Why UltraGrid?

UltraGrid is not just a simulation — it is a **high-performance engine for exploring decentralized energy systems at scale**.

If you care about:

* performance
* modern C++
* real-world simulations

👉 this project is for you.
