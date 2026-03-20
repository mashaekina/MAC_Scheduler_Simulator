#include "round_robin_scheduler.h"

namespace telecom {

std::vector<uint32_t> RoundRobinScheduler::schedule(std::deque<UE>& ueQueue, uint32_t availableRbs) {
    std::vector<uint32_t> allocations;
    if (ueQueue.empty() || availableRbs == 0) {
        return allocations;
    }

    const size_t ues = ueQueue.size();
    // Normalise on every call so lastIndex_ never drifts past queue size.
    lastIndex_ = lastIndex_ % ues;

    size_t checked = 0;
    while (availableRbs > 0 && checked < ues) {
        if (ueQueue[lastIndex_].pendingSize() > 0) {
            allocations.push_back(ueQueue[lastIndex_].id);
            --availableRbs;
        }
        lastIndex_ = (lastIndex_ + 1) % ues;
        ++checked;
    }
    return allocations;
}

} // namespace telecom
