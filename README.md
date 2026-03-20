# Telecom MAC Scheduler Simulator

A multithreaded downlink MAC scheduler simulator for 5G NR, implemented in C++17.
Covers the L1/L2 boundary: AWGN channel model, CQI→MCS mapping, HARQ retransmission,
Round Robin and Proportional Fair scheduling algorithms.

## Features

| Component | Details |
|---|---|
| **Schedulers** | Round Robin, Proportional Fair (EWMA-based) |
| **Channel model** | AWGN: SNR → CQI (3GPP-style table), CQI → MCS |
| **HARQ** | Chase combining, up to 4 retransmissions per process |
| **Multithreading** | Per-UE worker threads update CQI and generate traffic concurrently |
| **Metrics** | Throughput (RBs/TTI), Jain's Fairness Index — exported to CSV |
| **Visualisation** | Python script plots throughput and fairness from CSV |
| **Tests** | GTest unit tests for all major components |

## Project structure

```
.
├── include/
│   ├── scheduler.h                  # Abstract base template
│   ├── round_robin_scheduler.h
│   ├── proportional_fair_scheduler.h
│   ├── ue.h                         # User Equipment data model
│   ├── resource_block.h             # RAII ResourceBlockHandle
│   ├── channel_model.h              # AWGN, SNR→CQI, CQI→MCS
│   └── harq.h                       # HARQ process & manager
├── src/
│   ├── main.cpp                     # Simulation entry point
│   ├── round_robin_scheduler.cpp
│   ├── proportional_fair_scheduler.cpp
│   ├── channel_model.cpp
│   ├── harq.cpp
│   └── ...
├── tests/
│   └── test_scheduler.cpp           # GTest suite
├── python/
│   └── plot_metrics.py              # Matplotlib visualisation
└── CMakeLists.txt
```

## Build

Requires: CMake >= 3.16, C++17 compiler (GCC 9+, Clang 10+, MSVC 2019+).
GTest is fetched automatically via CMake FetchContent.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Run

```bash
./build/src/simulator
```

Runs 100 TTIs with Round Robin, then 100 TTIs with Proportional Fair.
Writes `metrics_rr.csv` and `metrics_pf.csv` to the working directory.

## Tests

```bash
cd build && ctest --output-on-failure
```

## Visualise

```bash
pip install matplotlib
python python/plot_metrics.py metrics_rr.csv --output rr.png
python python/plot_metrics.py metrics_pf.csv --output pf.png
```

## Algorithm notes

### Round Robin
Each TTI the scheduler continues from where it left off and assigns one RB per UE
until `availableRbs` is exhausted or all UEs have been visited once.

### Proportional Fair
Ranks UEs each TTI by the metric `CQI / avg_throughput`, where `avg_throughput`
is maintained as an EWMA (α = 0.1, ~10-TTI window).
This balances spectral efficiency with long-term fairness: UEs with good channels
are preferred, but no UE is starved indefinitely.

### HARQ
Up to 4 retransmission attempts per process (chase combining model).
The manager resets a process on ACK or after the maximum attempt count is reached.
