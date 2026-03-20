#pragma once

#include <cstdint>
#include <vector>

namespace telecom {

// Forward declarations
struct UE;

// Base scheduler concept.
// TQueue is the queue type used by the scheduler (e.g., std::deque<UE>). 
// The schedule() method must allocate ResourceBlocks to UEs for one TTI.

template <typename TQueue>
class Scheduler {
public:
    virtual ~Scheduler() = default;

    // Perform scheduling for a single TTI.
    // "availableRbs" represents the number of resource blocks available to allocate.
    // Returns a vector of UE ids that receive an allocation.
    virtual std::vector<uint32_t> schedule(TQueue& ueQueue, uint32_t availableRbs) = 0;
};

} // namespace telecom
