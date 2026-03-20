#include "harq.h"

namespace telecom {

bool HarqManager::requestRetransmission(HarqProcess& proc) {
    if (proc.retransmissionCount >= kMaxRetransmissions) {
        return false;
    }
    ++proc.retransmissionCount;
    return true;
}

void HarqManager::reset(HarqProcess& proc) {
    proc.retransmissionCount = 0;
    proc.acked = false;
}

} // namespace telecom
