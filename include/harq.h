#pragma once

#include <cstdint>
#include <optional>

namespace telecom {

struct HarqProcess {
    uint32_t ueId;
    uint32_t retransmissionCount = 0;
    bool acked = false;
};

class HarqManager {
public:
    // Request a retransmission; returns false if max attempts reached.
    bool requestRetransmission(HarqProcess& proc);

    // Reset the HARQ state after a successful transmission.
    void reset(HarqProcess& proc);

private:
    static constexpr uint32_t kMaxRetransmissions = 4;
};

} // namespace telecom
