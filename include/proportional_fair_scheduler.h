#pragma once

#include "scheduler.h"
#include "ue.h"

#include <cstdint>
#include <deque>
#include <unordered_map>
#include <vector>

namespace telecom {

// Proportional Fair scheduler.
// Tracks a per-UE exponentially weighted moving average (EWMA) of throughput
// across TTIs. Each TTI it ranks UEs by instantaneous_cqi / avg_throughput,
// giving priority to UEs with good channel conditions relative to their history.
class ProportionalFairScheduler : public Scheduler<std::deque<UE>> {
public:
    ProportionalFairScheduler();

    std::vector<uint32_t> schedule(std::deque<UE>& ueQueue, uint32_t availableRbs) override;

private:
    // EWMA smoothing factor (0 < alpha <= 1).
    // alpha = 0.1 corresponds roughly to a 10-TTI averaging window.
    static constexpr double kAlpha = 0.1;

    // Maps UE id -> EWMA average throughput (initialised to 1e-6 to avoid /0).
    std::unordered_map<uint32_t, double> stats_;
};

} // namespace telecom
