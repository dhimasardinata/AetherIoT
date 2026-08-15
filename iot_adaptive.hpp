#pragma once

/**
 * ============================================================================
 * ADAPTIVE SAMPLING & SENSOR HEALTH SCORING (iot_adaptive.hpp)
 * ============================================================================
 * 1. Adaptive Delta Sampling: Fast burst transmission on spikes, slow on steady state.
 * 2. Multi-Sensor Health Score Index (0-100%): Evaluates hardware buses and error rates.
 * ============================================================================
 */

#include <cstdint>
#include <cmath>
#include "config.hpp"
#include "iot_core.hpp"

namespace iot::adaptive {

struct SamplingThresholds {
    float delta_ph{0.20f};      // Transmit immediately if pH changes > 0.20
    float delta_temp{1.0f};     // Transmit immediately if Temp changes > 1.0 C
    float delta_ec{50.0f};      // Transmit immediately if EC changes > 50 uS/cm
    float delta_dist_mm{30.0f}; // Transmit immediately if water level changes > 30mm
};

class AdaptiveSamplingManager {
public:
    static bool should_transmit(const UnifiedTelemetry& current, uint32_t now_ms) noexcept {
        if (!enabled_) return true;

        const float ph = static_cast<float>(current.water_ph_mili) / 1000.0f;
        const float temp = static_cast<float>(current.air_temperature_centi_c) / 100.0f;
        const float ec = static_cast<float>(current.water_ec_us_cm);
        const float dist = static_cast<float>(current.water_distance_mm);

        const bool delta_exceeded = (std::abs(ph - last_ph_) >= thresholds_.delta_ph) ||
                                    (std::abs(temp - last_temp_) >= thresholds_.delta_temp) ||
                                    (std::abs(ec - last_ec_) >= thresholds_.delta_ec) ||
                                    (std::abs(dist - last_dist_) >= thresholds_.delta_dist_mm);

        const uint32_t max_interval_ms = 30000; // 30s heartbeat interval
        const bool time_expired = (now_ms - last_tx_ms_ >= max_interval_ms);

        if (delta_exceeded || time_expired) {
            last_tx_ms_ = now_ms;
            last_ph_ = ph;
            last_temp_ = temp;
            last_ec_ = ec;
            last_dist_ = dist;
            return true;
        }
        return false;
    }

    static void set_enabled(bool enable) noexcept { enabled_ = enable; }

private:
    static inline bool               enabled_{true};
    static inline SamplingThresholds thresholds_{};
    static inline uint32_t           last_tx_ms_{0};
    static inline float              last_ph_{0.0f};
    static inline float              last_temp_{0.0f};
    static inline float              last_ec_{0.0f};
    static inline float              last_dist_{0.0f};
};

class SystemHealthAnalyzer {
public:
    [[nodiscard]] static uint8_t compute_health_score(const UnifiedTelemetry& data) noexcept {
        uint8_t score = 100;

        // Deduct points for sensor invalid flags
        if (!data.flags.ph_valid && Config_Sensors_Modbus_ENABLE_PH) score -= 15;
        if (!data.flags.ec_valid && Config_Sensors_Modbus_ENABLE_EC) score -= 15;
        if (!data.flags.sht_valid && Config_Sensors_I2C_ENABLE_SHT)  score -= 10;
        if (!data.flags.level_valid) score -= 10;

        // Deduct points for CRC / bus error counts
        if (data.modbus_crc_errors > 5) score -= 15;
        if (data.i2c_bus_errors > 5) score -= 15;

        // Deduct points if offline
        if (!data.flags.wifi_online && !data.flags.cellular_online) score -= 10;

        return (score > 100) ? 0 : score;
    }

private:
    static constexpr bool Config_Sensors_Modbus_ENABLE_PH = AppConfig::Sensors::ModbusSlaves::ENABLE_PH;
    static constexpr bool Config_Sensors_Modbus_ENABLE_EC = AppConfig::Sensors::ModbusSlaves::ENABLE_EC;
    static constexpr bool Config_Sensors_I2C_ENABLE_SHT   = AppConfig::Sensors::I2CDevices::ENABLE_SHT3X;
};

} // namespace iot::adaptive
