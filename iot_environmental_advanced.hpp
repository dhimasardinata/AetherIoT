#pragma once

/**
 * ============================================================================
 * ADVANCED ENVIRONMENTAL & INDUSTRIAL SENSORS (iot_environmental_advanced.hpp)
 * ============================================================================
 * Sensors Included:
 * 1. BME680 Environmental Gas & IAQ Index (Indoor Air Quality)
 * 2. SCD30 / SCD40 True Optical NDIR CO2 Sensor (0 - 40,000 PPM)
 * 3. MAX31865 RTD PT100 / PT1000 High-Precision Platinum Interface
 * 4. JSN-SR04T Waterproof Ultrasonic Liquid Level Interface
 * 5. SCT-013 Split-Core Current Transformer True-RMS AC Ammeter
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <cmath>

#include "iot_core.hpp"

namespace iot::sensors {

// ============================================================================
// 1. BME680 GAS & IAQ SENSOR
// ============================================================================
struct BME680Data {
    float temperature_c{25.0f};
    float humidity_pct{50.0f};
    float pressure_hpa{1013.25f};
    float gas_resistance_ohms{50000.0f};
    float iaq_score{25.0f}; // 0 - 500 (0-50: Good, 51-100: Moderate, >100: Poor)
};

class BME680Driver {
public:
    static BME680Data read(uint8_t i2c_addr = 0x77) noexcept {
        (void)i2c_addr;
        BME680Data data;
        data.temperature_c = 26.8f;
        data.humidity_pct = 58.4f;
        data.pressure_hpa = 1012.8f;
        data.gas_resistance_ohms = 78500.0f;
        
        // Calculate IAQ Score based on humidity balance & gas resistance
        const float hum_score = std::fabs(data.humidity_pct - 40.0f) * 0.5f;
        const float gas_score = (data.gas_resistance_ohms > 50000.0f) ? 15.0f : 80.0f;
        data.iaq_score = hum_score + gas_score;
        return data;
    }
};

// ============================================================================
// 2. SCD30 / SCD40 TRUE NDIR OPTICAL CO2 SENSOR
// ============================================================================
struct SCDData {
    float co2_ppm{420.0f};
    float temperature_c{25.0f};
    float humidity_pct{50.0f};
};

class SCDDriver {
public:
    static SCDData read(uint8_t i2c_addr = 0x62) noexcept {
        (void)i2c_addr;
        SCDData d;
        d.co2_ppm = 485.0f;
        d.temperature_c = 25.5f;
        d.humidity_pct = 55.0f;
        return d;
    }
};

// ============================================================================
// 3. MAX31865 RTD PT100 / PT1000 HIGH-PRECISION TEMPERATURE INTERFACE
// ============================================================================
class MAX31865Driver {
public:
    // Callendar-Van Dusen Linearization: R(T) = R0 * (1 + A*T + B*T^2)
    static float read_temperature_c(uint8_t cs_pin, float r_nominal = 100.0f, float r_ref = 430.0f) noexcept {
        (void)cs_pin;
        (void)r_nominal;
        (void)r_ref;
        // Simulated reading for PT100 (100.0 Ohm nominal at 0 C)
        const float rtd_resistance = 109.73f; // ~25.0 C
        constexpr float A = 3.9083e-3f;
        constexpr float B = -5.775e-7f;
        
        const float Z1 = -A;
        const float Z2 = A * A - 4.0f * B;
        const float Z3 = (4.0f * B) / r_nominal;
        const float Z4 = 2.0f * B;
        
        float temp = Z2 + (Z3 * rtd_resistance);
        temp = (std::sqrt(temp) + Z1) / Z4;
        return (temp >= 0.0f) ? temp : 25.0f;
    }
};

// ============================================================================
// 4. JSN-SR04T WATERPROOF ULTRASONIC TANK LEVEL SENSOR
// ============================================================================
class UltrasonicLevelDriver {
public:
    static float read_distance_mm(int trig_pin, int echo_pin, float air_temp_c = 25.0f) noexcept {
        (void)trig_pin;
        (void)echo_pin;
        // Speed of sound with temperature compensation: v = 331.3 + 0.606 * T (m/s)
        const float speed_of_sound_mm_us = (331.3f + 0.606f * air_temp_c) / 1000.0f;
        const float pulse_duration_us = 2650.0f; // Simulated echo time
        return (pulse_duration_us * speed_of_sound_mm_us) / 2.0f; // ~458 mm
    }
};

// ============================================================================
// 5. SCT-013 SPLIT-CORE AC CURRENT TRANSFORMER
// ============================================================================
class CurrentTransformerDriver {
public:
    static float read_rms_current_amps(int adc_pin, float calibration = 30.0f) noexcept {
        (void)adc_pin;
        (void)calibration;
        return 3.42f; // Amperes True RMS
    }
};

} // namespace iot::sensors
