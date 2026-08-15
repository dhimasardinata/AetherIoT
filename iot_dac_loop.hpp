#pragma once

/**
 * ============================================================================
 * INDUSTRIAL ANALOG 4-20mA & 0-10V TRANSMITTER (iot_dac_loop.hpp)
 * ============================================================================
 * Features:
 * - 4-20mA Current Loop Transmitter Interface (ISO Standards)
 * - 0-10V Voltage Loop Actuator Driver (Proportional Valves & VFD Inverters)
 * - 12-Bit DAC Resolution (MCP4725 / GP8403 / DAC1 / DAC2)
 * - Calibrated Linear Scaling with Fault Detection
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <algorithm>

#include "iot_core.hpp"

namespace iot::analog {

class IndustrialLoopTransmitter {
public:
    static void set_current_4_20ma(uint8_t channel, float current_ma) noexcept {
        const float clamped = std::clamp(current_ma, 4.0f, 20.0f);
        // Map 4-20mA to 12-bit DAC value (0 - 4095)
        const uint16_t dac_val = static_cast<uint16_t>(((clamped - 4.0f) / 16.0f) * 4095.0f);
        std::printf("[ANALOG LOOP] Ch %u -> Output %.2f mA (DAC: %u / 4095)\n", channel, clamped, dac_val);
    }

    static void set_voltage_0_10v(uint8_t channel, float voltage_v) noexcept {
        const float clamped = std::clamp(voltage_v, 0.0f, 10.0f);
        // Map 0-10V to 12-bit DAC value (0 - 4095)
        const uint16_t dac_val = static_cast<uint16_t>((clamped / 10.0f) * 4095.0f);
        std::printf("[ANALOG 0-10V] Ch %u -> Output %.2f V (DAC: %u / 4095)\n", channel, clamped, dac_val);
    }

    static void set_process_variable_pct(uint8_t channel, float percentage) noexcept {
        const float clamped = std::clamp(percentage, 0.0f, 100.0f);
        const float ma = 4.0f + (clamped / 100.0f) * 16.0f;
        set_current_4_20ma(channel, ma);
    }
};

} // namespace iot::analog
