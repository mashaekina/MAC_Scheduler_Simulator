#pragma once

#include <cstdint>
#include <deque>
#include <mutex>

namespace telecom {

// A simple representation of a UE (User Equipment).
// In a full simulator this would contain buffers, CQI reporting, HARQ state, etc.
struct UE {
    uint32_t id;
    double lastReportedCqi = 0.0;
    std::deque<size_t> pendingPackets;

    // Thread-safe access to queue size (for scheduler decisions).
    size_t pendingSize() const {
        return pendingPackets.size();
    }
};

} // namespace telecom
