#pragma once

#include <cstdint>
#include <vector>

namespace telecom {

// Simple AWGN channel model.
// This is a toy implementation for simulator purposes.
class ChannelModel {
public:
    // Convert SNR (dB) to a CQI index (0..15) using a simple threshold table.
    static uint8_t snrToCqi(double snr_db);

    // Simple CQI -> MCS mapping (returns bits per symbol).
    static uint8_t cqiToMcs(uint8_t cqi);

    // Add gaussian noise to a signal power and return resulting SNR.
    static double addAwgn(double signalPowerDb, double noisePowerDb);
};

} // namespace telecom
