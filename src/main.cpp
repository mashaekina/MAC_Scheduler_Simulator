#include "channel_model.h"
#include "harq.h"
#include "proportional_fair_scheduler.h"
#include "round_robin_scheduler.h"
#include "ue.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <iostream>
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>

using namespace telecom;

// ---------------------------------------------------------------------------
// Simulation parameters
// ---------------------------------------------------------------------------
static constexpr int      kNumUes      = 5;
static constexpr int      kNumTtis     = 100;
static constexpr int      kTtiMs       = 1;
static constexpr uint32_t kAvailableRbs = 10;
static constexpr double   kNoisePower  = 5.0; // dB
// Different base SNR per UE to simulate heterogeneous channel conditions.
static const double kBaseSnr[kNumUes]  = {5.0, 10.0, 15.0, 20.0, 25.0};

// ---------------------------------------------------------------------------
// Shared UE queue (protected by mutex).
// ---------------------------------------------------------------------------
static std::deque<UE>    gUeQueue;
static std::mutex        gUeQueueMutex;
static std::atomic<bool> gRunning{true};

// ---------------------------------------------------------------------------
// UE worker thread: updates CQI and generates traffic every few TTIs.
// ---------------------------------------------------------------------------
static void ueWorker(uint32_t ueId, double baseSnrDb) {
    while (gRunning.load(std::memory_order_relaxed)) {
        // Small random fading variation (±2 dB).
        double variation = (static_cast<double>(std::rand() % 400) - 200.0) / 100.0;
        double snr       = ChannelModel::addAwgn(baseSnrDb + variation, kNoisePower);
        uint8_t cqi      = ChannelModel::snrToCqi(snr);

        {
            std::lock_guard<std::mutex> lock(gUeQueueMutex);
            for (auto& ue : gUeQueue) {
                if (ue.id == ueId) {
                    ue.lastReportedCqi = static_cast<double>(cqi);
                    ue.pendingPackets.push_back(1500); // bytes
                    break;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(kTtiMs * 5));
    }
}

// ---------------------------------------------------------------------------
// Jain's Fairness Index: J = (sum x_i)^2 / (n * sum x_i^2)
// ---------------------------------------------------------------------------
static double jainFairness(const std::vector<double>& allocs) {
    if (allocs.empty()) return 1.0;
    double sum   = std::accumulate(allocs.begin(), allocs.end(), 0.0);
    double sumSq = std::inner_product(allocs.begin(), allocs.end(), allocs.begin(), 0.0);
    if (sumSq < 1e-12) return 1.0;
    return (sum * sum) / (static_cast<double>(allocs.size()) * sumSq);
}

// ---------------------------------------------------------------------------
// Run a scheduling simulation and write per-TTI metrics to a CSV file.
// ---------------------------------------------------------------------------
template <typename TScheduler>
static void runSimulation(TScheduler& scheduler,
                          const std::string& label,
                          const std::string& csvPath) {
    std::ofstream csv(csvPath);
    csv << "tti,scheduler,throughput_rb,fairness\n";

    std::vector<double> ueAllocs(kNumUes, 0.0); // cumulative RB count per UE

    HarqManager harqMgr;
    std::vector<HarqProcess> harqProcs(kNumUes);
    for (uint32_t i = 0; i < kNumUes; ++i) {
        harqProcs[i].ueId = i + 1;
    }

    for (int tti = 0; tti < kNumTtis; ++tti) {
        std::vector<uint32_t> scheduled;
        {
            std::lock_guard<std::mutex> lock(gUeQueueMutex);
            scheduled = scheduler.schedule(gUeQueue, kAvailableRbs);

            // Consume one packet per scheduled UE.
            for (uint32_t id : scheduled) {
                for (auto& ue : gUeQueue) {
                    if (ue.id == id && !ue.pendingPackets.empty()) {
                        ue.pendingPackets.pop_front();
                        break;
                    }
                }
            }
        }

        // HARQ: attempt retransmission for unscheduled UEs with pending NACKs.
        for (auto& proc : harqProcs) {
            bool wasScheduled = std::find(scheduled.begin(), scheduled.end(),
                                          proc.ueId) != scheduled.end();
            if (wasScheduled) {
                harqMgr.reset(proc); // treat as ACK
            } else if (!proc.acked && proc.retransmissionCount > 0) {
                harqMgr.requestRetransmission(proc); // increments counter or gives up
            }
        }

        // Update per-UE cumulative allocations.
        for (uint32_t id : scheduled) {
            if (id >= 1 && id <= static_cast<uint32_t>(kNumUes)) {
                ueAllocs[id - 1] += 1.0;
            }
        }

        double fairness     = jainFairness(ueAllocs);
        uint32_t throughput = static_cast<uint32_t>(scheduled.size());

        csv << tti << "," << label << "," << throughput << "," << fairness << "\n";

        std::cout << "TTI " << tti << " [" << label << "] allocated:";
        for (auto id : scheduled) std::cout << " UE" << id;
        std::cout << "  fairness=" << fairness << "\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(kTtiMs));
    }

    std::cout << "Metrics written to " << csvPath << "\n";
}

int main() {
    // Initialise UE queue with initial packet backlog.
    for (uint32_t id = 1; id <= kNumUes; ++id) {
        UE ue;
        ue.id              = id;
        ue.lastReportedCqi = static_cast<double>(ChannelModel::snrToCqi(kBaseSnr[id - 1]));
        ue.pendingPackets  = std::deque<size_t>(10, 1500);
        gUeQueue.push_back(std::move(ue));
    }

    // Start per-UE threads that simulate CQI reporting and traffic generation.
    std::vector<std::thread> ueThreads;
    ueThreads.reserve(kNumUes);
    for (uint32_t id = 1; id <= kNumUes; ++id) {
        ueThreads.emplace_back(ueWorker, id, kBaseSnr[id - 1]);
    }

    std::cout << "=== Round Robin ===\n";
    RoundRobinScheduler rr;
    runSimulation(rr, "RR", "metrics_rr.csv");

    // Reset UE packet queues so PF starts under the same conditions as RR.
    {
        std::lock_guard<std::mutex> lock(gUeQueueMutex);
        for (auto& ue : gUeQueue) {
            ue.pendingPackets = std::deque<size_t>(10, 1500);
        }
    }

    std::cout << "\n=== Proportional Fair ===\n";
    ProportionalFairScheduler pf;
    runSimulation(pf, "PF", "metrics_pf.csv");

    gRunning.store(false);
    for (auto& t : ueThreads) t.join();

    return 0;
}
