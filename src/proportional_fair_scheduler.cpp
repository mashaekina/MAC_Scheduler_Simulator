#include "proportional_fair_scheduler.h"

#include <algorithm>

namespace telecom {

ProportionalFairScheduler::ProportionalFairScheduler() = default;

std::vector<uint32_t> ProportionalFairScheduler::schedule(std::deque<UE>& ueQueue, uint32_t availableRbs) {
    std::vector<uint32_t> allocations;
    if (ueQueue.empty() || availableRbs == 0) {
        return allocations;
    }

    // Ensure every UE has an EWMA entry (new UEs start at a tiny value to
    // avoid division by zero while not unfairly dominating the metric).
    for (const auto& ue : ueQueue) {
        if (stats_.find(ue.id) == stats_.end()) {
            stats_[ue.id] = 1e-6;
        }
    }

    // PF metric: instantaneous_rate / average_throughput.
    // Instantaneous rate is approximated by lastReportedCqi.
    struct Candidate { uint32_t id; double metric; };
    std::vector<Candidate> candidates;
    candidates.reserve(ueQueue.size());

    for (const auto& ue : ueQueue) {
        double metric = (ue.lastReportedCqi + 1e-9) / stats_.at(ue.id);
        candidates.push_back({ue.id, metric});
    }

    std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) {
        return a.metric > b.metric;
    });

    for (const auto& candidate : candidates) {
        if (availableRbs == 0) break;

        auto it = std::find_if(ueQueue.begin(), ueQueue.end(),
                               [&](const auto& ue) { return ue.id == candidate.id; });
        if (it != ueQueue.end() && it->pendingSize() > 0) {
            allocations.push_back(candidate.id);
            --availableRbs;
            // EWMA update for scheduled UE.
            stats_[candidate.id] = (1.0 - kAlpha) * stats_[candidate.id]
                                   + kAlpha * it->lastReportedCqi;
        }
    }

    // Decay average for UEs that were NOT scheduled this TTI.
    for (auto& [id, avg] : stats_) {
        bool scheduled = std::find(allocations.begin(), allocations.end(), id) != allocations.end();
        if (!scheduled) {
            avg = (1.0 - kAlpha) * avg;
        }
    }

    return allocations;
}

} // namespace telecom
