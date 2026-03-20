#include "channel_model.h"

#include <cmath>

namespace telecom {

uint8_t ChannelModel::snrToCqi(double snr_db) {
    // Simplified mapping from SNR to CQI for demonstration.
    // Values are based on typical 3GPP thresholds but are not exact.
    if (snr_db < -5.0) return 0;
    if (snr_db < 0.0) return 1;
    if (snr_db < 2.5) return 2;
    if (snr_db < 5.0) return 3;
    if (snr_db < 7.5) return 4;
    if (snr_db < 10.0) return 5;
    if (snr_db < 12.5) return 6;
    if (snr_db < 15.0) return 7;
    if (snr_db < 17.5) return 8;
    if (snr_db < 20.0) return 9;
    if (snr_db < 22.5) return 10;
    if (snr_db < 25.0) return 11;
    if (snr_db < 27.5) return 12;
    if (snr_db < 30.0) return 13;
    if (snr_db < 32.5) return 14;
    return 15;
}

uint8_t ChannelModel::cqiToMcs(uint8_t cqi) {
    // Very simplified mapping: MCS index ~ CQI.
    // In 3GPP, CQI to MCS is table-based and depends on bandwidth
    // and target BLER. This placeholder returns a modulation order.
    return std::min<uint8_t>(15, cqi);
}

// AWGN: simple subtraction in dB scale.
// In reality, SNR = signal / noise.
double ChannelModel::addAwgn(double signalPowerDb, double noisePowerDb) {
    return signalPowerDb - noisePowerDb;
}

} // namespace telecom
