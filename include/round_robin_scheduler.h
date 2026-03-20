#pragma once

#include "scheduler.h"
#include "ue.h"

#include <cstdint>
#include <deque>
#include <vector>

namespace telecom {

// Simple Round-Robin scheduler.
// Rotates through the UEs in the queue and assigns one RB per active UE until
// the available resource blocks are exhausted.
class RoundRobinScheduler : public Scheduler<std::deque<UE>> {
public:
    RoundRobinScheduler() = default;

    std::vector<uint32_t> schedule(std::deque<UE>& ueQueue, uint32_t availableRbs) override;

private:
    // tracks the index of the next UE to schedule
    size_t lastIndex_ = 0;
};

} // namespace telecom
