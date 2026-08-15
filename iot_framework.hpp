#pragma once

/**
 * ============================================================================
 * UNIFIED HIGH-PERFORMANCE IOT FRAMEWORK (iot_framework.hpp)
 * ============================================================================
 * Enterprise Master Suite:
 * - Multi-Target Espressif Support: ESP32 Classic, S2, S3, C2, C3, C6, H2, P4, ESP8266.
 * - High-Density Multi-Servo Matrix (Up to 32 Servos with Relay-Like Open/Close API).
 * - High-Density Sensor Multiplexing:
 *   - TCA9548A 8-Channel I2C Multiplexer
 *   - 1-Wire DS18B20 Multi-Drop Temperature Probe Array (16-32 Probes on 1 GPIO)
 *   - CD74HC4067 16-Channel Analog MUX & ADS1115 16-Bit I2C ADC
 *   - Modbus RS-485 Multi-Slave Polling Hub (up to 32 Slaves)
 * - Tipping Bucket Rain Gauge & Weather Station Integration.
 * - Bluetooth Classic SPP & BLE Nordic UART Service (NUS) Serial Monitor & BLE OTA.
 * - Tiered Offline Caching (RAM Ring Buffer -> Flash Spool -> MicroSD Card -> Cloud Backfill).
 * - High-Density Actuator Expanders: PCA9685 16-Ch 12-Bit PWM, 74HC595 16-64 Relay Matrix.
 * - Bistable Latching Solenoid Pulse Control (Zero Holding Power).
 * - Thread / Zigbee / Matter 802.15.4 Mesh Radio on ESP32-C6 & ESP32-H2.
 * - 16-Channel Relays with Custom Names & Inrush Staggering.
 * - Long-Range LoRa SX1276/SX1278 (433/868/915 MHz) & Multi-Node Cluster Registry.
 * - MicroSD FAT32 CSV Telemetry Logger with Daily File Rotation.
 * - Multi-SSID Priority WiFi Fallback Store (Primary, Backup, Hotspot).
 * - Gateway Control State: AUTO, MANUAL_OVERRIDE, SAFETY_LOCKOUT.
 * - Weekly Day Bitmask Scheduler (Days::MON | Days::WED | Days::FRI).
 * - MPU6050 6-Axis IMU, Pitch/Roll Tilt Angles & Vibration Anomaly Guard.
 * - Captive Portal DNS Server (Auto-Popup Setup on Phone Connect).
 * - Multi-Scheme Payload Encryption (AES-128, AES-256-CBC Anti-Replay, ChaCha20, HMAC Envelope).
 * - Safety Range Guards & Anomaly Watchdogs (`iot::guard(...).between(...).on_breach(...)`).
 * - Deep Sleep (< 15 uA) & Battery Fuel Gauge Calculation.
 * - Pulse Flow Meter (L/min & Cumulative Liters), Servo & Stepper Dispenser.
 * - BLE GATT Smartphone Provisioning & Auto-APN Detection.
 * - Adaptive Sampling & Multi-Sensor Hardware Health Score (0-100%).
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <string_view>
#include <array>

#include "config.hpp"
#include "iot_target.hpp"
#include "iot_core.hpp"
#include "iot_crypto.hpp"
#include "iot_storage.hpp"
#include "iot_espnow.hpp"
#include "iot_drivers.hpp"
#include "iot_system.hpp"
#include "iot_ota.hpp"
#include "iot_security.hpp"
#include "iot_web.hpp"
#include "iot_cellular.hpp"
#include "iot_lcd.hpp"
#include "iot_protocols.hpp"
#include "iot_diagnostics.hpp"
#include "iot_automation.hpp"
#include "iot_adaptive.hpp"
#include "iot_ble.hpp"
#include "iot_guards.hpp"
#include "iot_power.hpp"
#include "iot_actuators_advanced.hpp"
#include "iot_actuators_expanded.hpp"
#include "iot_multi_servo.hpp"
#include "iot_sensor_multiplexer.hpp"
#include "iot_lora.hpp"
#include "iot_imu.hpp"
#include "iot_captive_dns.hpp"
#include "iot_mesh_registry.hpp"
#include "iot_sd_logger.hpp"
#include "iot_wifi_multi.hpp"
#include "iot_control_mode.hpp"
#include "iot_thread_matter.hpp"
#include "iot_rain_gauge.hpp"
#include "iot_ble_serial_ota.hpp"
#include "iot_caching_tiered.hpp"
#include "iot_cli.hpp"
#include "iot_environmental_advanced.hpp"
#include "iot_stepper_motor.hpp"
#include "iot_dac_loop.hpp"
#include "iot_partitions.hpp"
#include "iot_indicator.hpp"
#include "iot_mqtt_custom.hpp"
#include "iot_button.hpp"
#include "iot_modbus_rtu.hpp"
#include "iot_http_webhook.hpp"

namespace iot {

// ============================================================================
// 1. COMPILE-TIME ACTUATOR ALIASES & RELAY ENUMS (16 CHANNELS)
// ============================================================================
namespace Relay {
    enum class Channel : uint8_t {
        CH1 = 1, CH2 = 2, CH3 = 3, CH4 = 4,
        CH5 = 5, CH6 = 6, CH7 = 7, CH8 = 8,
        CH9 = 9, CH10 = 10, CH11 = 11, CH12 = 12,
        CH13 = 13, CH14 = 14, CH15 = 15, CH16 = 16,
        VALVE_MAIN      = 9,
        PUMP_MAIN       = 10,
        FAN_EXHAUST     = 11,
        HEATER_MAIN     = 12,
        COOLER_MAIN     = 13,
        MOTOR_DRIVE     = 14,
        ALARM_BUZZER    = 15,
        STATUS_LAMP     = 16,
        SOLENOID_INLET  = 1,
        SOLENOID_OUTLET = 2,
        PUMP_DOSING_A   = 3,
        PUMP_DOSING_B   = 4,
        AERATOR_MAIN    = 5,
        FEEDER_ACTUATOR = 6
    };

    // Generic Direct Channel Numbers
    inline constexpr uint8_t CH1 = 1;
    inline constexpr uint8_t CH2 = 2;
    inline constexpr uint8_t CH3 = 3;
    inline constexpr uint8_t CH4 = 4;
    inline constexpr uint8_t CH5 = 5;
    inline constexpr uint8_t CH6 = 6;
    inline constexpr uint8_t CH7 = 7;
    inline constexpr uint8_t CH8 = 8;
    inline constexpr uint8_t CH9 = 9;
    inline constexpr uint8_t CH10 = 10;
    inline constexpr uint8_t CH11 = 11;
    inline constexpr uint8_t CH12 = 12;
    inline constexpr uint8_t CH13 = 13;
    inline constexpr uint8_t CH14 = 14;
    inline constexpr uint8_t CH15 = 15;
    inline constexpr uint8_t CH16 = 16;

    // Underscore aliases
    inline constexpr uint8_t CH_1  = 1;
    inline constexpr uint8_t CH_2  = 2;
    inline constexpr uint8_t CH_3  = 3;
    inline constexpr uint8_t CH_4  = 4;
    inline constexpr uint8_t CH_5  = 5;
    inline constexpr uint8_t CH_6  = 6;
    inline constexpr uint8_t CH_7  = 7;
    inline constexpr uint8_t CH_8  = 8;
    inline constexpr uint8_t CH_9  = 9;
    inline constexpr uint8_t CH_10 = 10;
    inline constexpr uint8_t CH_11 = 11;
    inline constexpr uint8_t CH_12 = 12;
    inline constexpr uint8_t CH_13 = 13;
    inline constexpr uint8_t CH_14 = 14;
    inline constexpr uint8_t CH_15 = 15;
    inline constexpr uint8_t CH_16 = 16;

    // Functional Industrial Aliases
    inline constexpr uint8_t VALVE_MAIN       = AppConfig::Actuators::Alias::VALVE_MAIN;
    inline constexpr uint8_t PUMP_MAIN        = AppConfig::Actuators::Alias::PUMP_MAIN;
    inline constexpr uint8_t FAN_EXHAUST      = AppConfig::Actuators::Alias::FAN_EXHAUST;
    inline constexpr uint8_t HEATER_MAIN      = AppConfig::Actuators::Alias::HEATER_MAIN;
    inline constexpr uint8_t COOLER_MAIN      = AppConfig::Actuators::Alias::COOLER_MAIN;
    inline constexpr uint8_t MOTOR_DRIVE      = AppConfig::Actuators::Alias::MOTOR_DRIVE;
    inline constexpr uint8_t ALARM_BUZZER     = AppConfig::Actuators::Alias::ALARM_BUZZER;
    inline constexpr uint8_t STATUS_LAMP      = AppConfig::Actuators::Alias::STATUS_LAMP;
    inline constexpr uint8_t SOLENOID_INLET   = 1;
    inline constexpr uint8_t SOLENOID_OUTLET  = 2;
    inline constexpr uint8_t PUMP_DOSING_A    = 3;
    inline constexpr uint8_t PUMP_DOSING_B    = 4;
    inline constexpr uint8_t AERATOR_MAIN     = 5;
    inline constexpr uint8_t FEEDER_ACTUATOR  = 6;
}

namespace Security {
    inline constexpr auto NONE        = security::EncryptionScheme::NONE;
    inline constexpr auto XOR_STREAM  = security::EncryptionScheme::XOR_STREAM;
    inline constexpr auto XOR_ROLLING = security::EncryptionScheme::XOR_STREAM;
    inline constexpr auto AES_128_CBC = security::EncryptionScheme::AES_128_CBC;
    inline constexpr auto AES_256_CBC = security::EncryptionScheme::AES_256_CBC;
    inline constexpr auto CHACHA20    = security::EncryptionScheme::CHACHA20;
    inline constexpr auto HMAC_SIGNED = security::EncryptionScheme::HMAC_SIGNED;
}

namespace Days {
    inline constexpr uint8_t MON = automation::Days::MON;
    inline constexpr uint8_t TUE = automation::Days::TUE;
    inline constexpr uint8_t WED = automation::Days::WED;
    inline constexpr uint8_t THU = automation::Days::THU;
    inline constexpr uint8_t FRI = automation::Days::FRI;
    inline constexpr uint8_t SAT = automation::Days::SAT;
    inline constexpr uint8_t SUN = automation::Days::SUN;
    inline constexpr uint8_t ALL = automation::Days::ALL;
}

namespace Control {
    inline constexpr auto AUTO            = control::Mode::AUTO;
    inline constexpr auto MANUAL_OVERRIDE = control::Mode::MANUAL_OVERRIDE;
    inline constexpr auto SAFETY_LOCKOUT  = control::Mode::SAFETY_LOCKOUT;
}

// Dynamic Custom Name Aliases Dictionary (Zero Heap)
class ActuatorRegistry {
public:
    static void set_alias(uint8_t channel, std::string_view name) noexcept {
        if (channel < 1 || channel > 16) return;
        names_[channel - 1].assign(name);
    }

    [[nodiscard]] static uint8_t find_channel(std::string_view name) noexcept {
        for (uint8_t i = 0; i < 16; ++i) {
            if (names_[i].string_view() == name) return i + 1;
        }
        return 0;
    }

private:
    static inline std::array<FixedString<24>, 16> names_{};
};

// ============================================================================
// 2. ERGONOMIC SENSOR DATA STRUCTURE (NATURAL UNITS & AUTO-COMPENSATED)
// ============================================================================
struct SensorData {
    // Air & Climate Environment
    float temp{0.0f};        // Air Temperature (Celsius, e.g. 28.5)
    float hum{0.0f};         // Air Humidity (%RH, e.g. 65.4)
    float lux{0.0f};         // Ambient Light (Lux, e.g. 450.0)
    float pressure{0.0f};    // Barometric Pressure (hPa, e.g. 1013.2)
    float dust_pm25{0.0f};   // PM2.5 Dust Particle (ug/m3)

    // Rainfall & Weather Station
    float rain_rate_mm_h{0.0f}; // Instantaneous Rainfall Rate (mm/h)
    float rain_daily_mm{0.0f};  // Daily Cumulative Rainfall (mm)

    // Water Quality & Nutrition (Temperature-Compensated)
    float ph{7.0f};          // Water pH (Nernst-Compensated, e.g. 6.85)
    float ec{0.0f};          // Water EC (Standard EC25 Compensated, uS/cm, e.g. 1850.0)
    float tds{0.0f};         // Water TDS (PPM, e.g. 920.0)
    float salinity{0.0f};    // Water Salinity (PPT, e.g. 1.2)
    float oxygen{0.0f};      // Dissolved Oxygen (mg/L or PPM, e.g. 6.5)
    float water_temp{0.0f};  // Water Temperature (Celsius, e.g. 26.0)

    // Liquid Tank Geometry & Flow
    float distance{0.0f};    // Distance to Liquid Surface (mm, e.g. 450.0)
    float water_height{0.0f};// Water Column Height (mm, e.g. 550.0)
    float volume{0.0f};      // Remaining Volume (Liters, e.g. 175.0)
    float flow_lpm{0.0f};    // Liquid Flow Rate (L/min)
    float total_flow{0.0f};  // Cumulative Total Flow (Liters)
    int   tank_pct{0};       // Tank Capacity Percentage (0 - 100 %)

    // IMU 6-Axis Motion & Structural Tilt (MPU6050)
    float tilt_pitch{0.0f};  // Pitch Tilt Angle (degrees)
    float tilt_roll{0.0f};   // Roll Tilt Angle (degrees)
    float vibration_g{0.0f}; // Structural Vibration (g)

    // Soil & Precision Agriculture (7-in-1 Probe)
    float soil_temp{0.0f};   // Soil Temperature (Celsius)
    float soil_hum{0.0f};    // Soil Moisture (%RH)
    float soil_ec{0.0f};     // Soil EC (uS/cm)
    float soil_ph{7.0f};     // Soil pH
    int   nitrogen{0};       // Nitrogen N (mg/kg)
    int   phosphorus{0};     // Phosphorus P (mg/kg)
    int   potassium{0};      // Potassium K (mg/kg)

    // Power, Voltage, Current & Battery Fuel Gauge
    float voltage{0.0f};     // Voltage (Volts, e.g. 12.4V or 220.0V)
    float current{0.0f};     // Current (Amperes, e.g. 1.25A)
    float power{0.0f};       // Power (Watts, e.g. 15.5W)
    float energy{0.0f};      // Cumulative Energy (kWh)
    uint8_t battery_pct{100};// Battery State of Charge (0 - 100 %)

    // GPS Positioning & GNSS
    float latitude{0.0f};
    float longitude{0.0f};
    int   satellites{0};

    // System Health & Telemetry Flags
    uint8_t health_pct{100}; // Bus & Hardware Health Score (0 - 100 %)
    bool  online{false};
    bool  ph_ok{false};
    bool  ec_ok{false};
    bool  level_ok{false};
    bool  emergency{false};
};

// ============================================================================
// 3. ERGONOMIC BUILT-IN LOGGING
// ============================================================================

template <typename... Args>
inline void log(const char* fmt, Args... args) noexcept {
    std::printf(fmt, args...);
    std::printf("\n");
}

template <typename... Args>
inline void log_info(const char* fmt, Args... args) noexcept {
    std::printf("\033[1;32m[INFO] \033[0m");
    std::printf(fmt, args...);
    std::printf("\n");
}

template <typename... Args>
inline void log_warn(const char* fmt, Args... args) noexcept {
    std::printf("\033[1;33m[WARN] \033[0m");
    std::printf(fmt, args...);
    std::printf("\n");
}

template <typename... Args>
inline void log_error(const char* fmt, Args... args) noexcept {
    std::printf("\033[1;31m[ERROR] \033[0m");
    std::printf(fmt, args...);
    std::printf("\n");
}

// ============================================================================
// 4. STANDALONE SENSORS & MULTIPLEXING
// ============================================================================

inline float read_ph() noexcept {
    auto res = drivers::ModbusIndustrialSensors<AppConfig>::read_ph_mili();
    if (res.is_err()) return 0.0f;
    const int32_t cal = CalibrationEngine<AppConfig>::instance().calibrate_ph(res.value());
    return static_cast<float>(cal) / 1000.0f;
}

inline float read_ec() noexcept {
    auto res = drivers::ModbusIndustrialSensors<AppConfig>::read_ec();
    if (res.is_err()) return 0.0f;
    return static_cast<float>(CalibrationEngine<AppConfig>::instance().calibrate_ec(res.value().ec_us_cm));
}

inline float read_lux() noexcept {
    auto res = drivers::BH1750Driver<AppConfig>::read_lux();
    return res.is_ok() ? static_cast<float>(res.value()) : 0.0f;
}

inline float read_temp() noexcept {
    auto res = drivers::SHT3xDriver<AppConfig>::read();
    return res.is_ok() ? (static_cast<float>(res.value().temp_centi_c) / 100.0f) : 0.0f;
}

inline float read_hum() noexcept {
    auto res = drivers::SHT3xDriver<AppConfig>::read();
    return res.is_ok() ? (static_cast<float>(res.value().hum_centi_rh) / 100.0f) : 0.0f;
}

inline float read_rain_rate() noexcept {
    return sensors::PulseRainGauge::rainfall_rate_mm_per_hour();
}

inline float read_daily_rain() noexcept {
    return sensors::PulseRainGauge::daily_rainfall_mm();
}

inline void rain_tip_pulse() noexcept {
    sensors::PulseRainGauge::record_tip();
}

// Multiplexed Sensor Functions
inline void i2c_channel(uint8_t channel_0_7) noexcept {
    multiplexer::I2CMultiplexerTCA9548A::select_channel(channel_0_7);
}

inline float read_temp_probe(size_t probe_index) noexcept {
    return multiplexer::OneWireMultiProbeArray<16>::read_temperature(probe_index);
}

inline uint16_t read_analog_mux(uint8_t channel_0_15) noexcept {
    return multiplexer::AnalogMultiplexerCD74HC4067::read_raw_adc(channel_0_15);
}

inline float read_analog_mux_voltage(uint8_t channel_0_15) noexcept {
    return multiplexer::AnalogMultiplexerCD74HC4067::read_voltage(channel_0_15);
}

inline uint16_t read_modbus_slave(uint8_t slave_id, uint16_t reg_addr) noexcept {
    auto res = multiplexer::ModbusMultiSlaveHub::read_holding_register(slave_id, reg_addr);
    return res.is_ok() ? res.value() : 0;
}

struct TankInfo {
    float liters{0.0f};
    int   percent{0};
    float height_mm{0.0f};
};

inline TankInfo read_tank(int distance_mm = 450) noexcept {
    const auto m = drivers::UltrasonicTankDriver<AppConfig>::calculate_metrics(distance_mm);
    TankInfo t{};
    t.liters = static_cast<float>(m.volume_liters);
    t.percent = m.percentage_full;
    t.height_mm = static_cast<float>(m.water_height_mm);
    return t;
}

// ============================================================================
// 5. 16-CHANNEL ACTUATOR, EXPANDER & MULTI-SERVO COMMANDS
// ============================================================================

inline void set_relay_name(uint8_t channel, std::string_view custom_name) noexcept {
    ActuatorRegistry::set_alias(channel, custom_name);
}

inline void on(uint8_t channel) noexcept {
    if (control::ControlStateManager::is_lockout()) return;
    ActuatorEngine<AppConfig>::set_relay(channel, true);
}

inline void off(uint8_t channel) noexcept {
    ActuatorEngine<AppConfig>::set_relay(channel, false);
}

inline void toggle(uint8_t channel) noexcept {
    if (control::ControlStateManager::is_lockout()) return;
    ActuatorEngine<AppConfig>::toggle_relay(channel);
}

[[nodiscard]] inline bool is_on(uint8_t channel) noexcept {
    return ActuatorEngine<AppConfig>::get_relay_state(channel);
}

// Strongly-Typed Relay::Channel Overloads
inline void on(Relay::Channel channel) noexcept { on(static_cast<uint8_t>(channel)); }
inline void off(Relay::Channel channel) noexcept { off(static_cast<uint8_t>(channel)); }
inline void toggle(Relay::Channel channel) noexcept { toggle(static_cast<uint8_t>(channel)); }
[[nodiscard]] inline bool is_on(Relay::Channel channel) noexcept { return is_on(static_cast<uint8_t>(channel)); }

// Explicit relay_ prefixed aliases
inline void relay_on(uint8_t channel) noexcept { on(channel); }
inline void relay_off(uint8_t channel) noexcept { off(channel); }
inline void relay_toggle(uint8_t channel) noexcept { toggle(channel); }
[[nodiscard]] inline bool relay_is_on(uint8_t channel) noexcept { return is_on(channel); }

inline void relay_on(Relay::Channel channel) noexcept { on(channel); }
inline void relay_off(Relay::Channel channel) noexcept { off(channel); }
inline void relay_toggle(Relay::Channel channel) noexcept { toggle(channel); }
[[nodiscard]] inline bool relay_is_on(Relay::Channel channel) noexcept { return is_on(channel); }

inline void on(std::string_view name) noexcept {
    const uint8_t ch = ActuatorRegistry::find_channel(name);
    if (ch > 0) on(ch);
}

inline void off(std::string_view name) noexcept {
    const uint8_t ch = ActuatorRegistry::find_channel(name);
    if (ch > 0) off(ch);
}

inline void toggle(std::string_view name) noexcept {
    const uint8_t ch = ActuatorRegistry::find_channel(name);
    if (ch > 0) toggle(ch);
}

// High-Density Multi-Servo (Declarative Relay-Like API)
inline void servo_name(size_t index, std::string_view name) noexcept {
    actuators::MultiServoMatrix<32>::set_alias(index, name);
}

inline void servo_open(size_t index) noexcept {
    if (control::ControlStateManager::is_lockout()) return;
    actuators::MultiServoMatrix<32>::open(index);
}

inline void servo_close(size_t index) noexcept {
    actuators::MultiServoMatrix<32>::close(index);
}

inline void servo_toggle(size_t index) noexcept {
    if (control::ControlStateManager::is_lockout()) return;
    actuators::MultiServoMatrix<32>::toggle(index);
}

inline void servo_on(size_t index) noexcept { servo_open(index); }
inline void servo_off(size_t index) noexcept { servo_close(index); }

inline void servo_open(std::string_view name) noexcept {
    if (control::ControlStateManager::is_lockout()) return;
    actuators::MultiServoMatrix<32>::open(name);
}

inline void servo_close(std::string_view name) noexcept {
    actuators::MultiServoMatrix<32>::close(name);
}

inline void servo_toggle(std::string_view name) noexcept {
    if (control::ControlStateManager::is_lockout()) return;
    actuators::MultiServoMatrix<32>::toggle(name);
}

inline void servo_set_angle(size_t index, float degrees) noexcept {
    if (control::ControlStateManager::is_lockout()) return;
    actuators::MultiServoMatrix<32>::set_angle(index, degrees);
}

inline void servo_set_calib(size_t index, float closed_deg, float open_deg) noexcept {
    actuators::MultiServoMatrix<32>::set_calibration(index, closed_deg, open_deg);
}

[[nodiscard]] inline bool servo_is_open(size_t index) noexcept {
    return actuators::MultiServoMatrix<32>::is_open(index);
}

inline void dim_pwm(uint8_t channel_0_15, float percent) noexcept {
    if (control::ControlStateManager::is_lockout()) return;
    actuators::PCA9685PWMExpander<AppConfig>::set_channel_pct(channel_0_15, percent);
}

inline void shift_relay(size_t index, bool state) noexcept {
    if (control::ControlStateManager::is_lockout()) return;
    actuators::ShiftRegisterRelayMatrix<64>::set_relay(index, state);
}

inline void latch_open(uint8_t open_pin, uint32_t pulse_ms = 50) noexcept {
    if (control::ControlStateManager::is_lockout()) return;
    actuators::LatchingSolenoidDriver::pulse_open(open_pin, pulse_ms);
}

inline void latch_close(uint8_t close_pin, uint32_t pulse_ms = 50) noexcept {
    actuators::LatchingSolenoidDriver::pulse_close(close_pin, pulse_ms);
}

inline void staggered_on(std::span<const uint8_t> channels, uint32_t delay_ms = 150) noexcept {
    if (control::ControlStateManager::is_lockout()) return;
    automation::StaggeredRelaySequencer<AppConfig>::energize_staggered(channels, delay_ms);
}

inline void dose_ml(uint8_t pump_channel, float ml) noexcept {
    if (control::ControlStateManager::is_lockout()) return;
    if (pump_channel < 1 || pump_channel > 16) return;
    if (!DosingBudgetManager<AppConfig>::instance().can_dose(ml)) {
        log_warn("Dosis %.1f mL dibatalkan: Kuota harian habis!", ml);
        return;
    }
    DosingBudgetManager<AppConfig>::instance().record_dose(ml);
    ActuatorEngine<AppConfig>::set_relay(pump_channel, true);
    log_info("Dosis %.1f mL dimulai pada channel %u", ml, pump_channel);
}

inline void emergency_stop() noexcept {
    control::ControlStateManager::set_mode(control::Mode::SAFETY_LOCKOUT);
    for (uint8_t ch = 1; ch <= 16; ++ch) {
        ActuatorEngine<AppConfig>::set_relay(ch, false);
    }
    actuators::ShiftRegisterRelayMatrix<64>::all_off();
    log_error("EMERGENCY STOP AKTIF: Seluruh aktuator dimatikan & dikunci!");
}

inline void set_control_mode(control::Mode m) noexcept {
    control::ControlStateManager::set_mode(m);
}

inline void add_wifi_ap(std::string_view ssid, std::string_view password, uint8_t priority = 0) noexcept {
    network::MultiWiFiStore::add_ap(ssid, password, priority);
}

inline void servo_angle(uint8_t degrees) noexcept {
    actuators::ServoController::set_angle(degrees);
}

inline void feed_grams(float grams) noexcept {
    actuators::StepperFeeder::dispense_grams(grams);
}

inline void deep_sleep(uint32_t seconds) noexcept {
    power::SleepManager::enter_deep_sleep(seconds);
}

inline void lora_send(uint16_t target_id, std::string_view payload) noexcept {
    lora::LoRaTransceiver<AppConfig>::send_packet(target_id, payload);
}

inline void lora_send(std::string_view payload) noexcept {
    lora::LoRaTransceiver<AppConfig>::send_packet(0xFFFF, payload);
}

inline void run_diagnostics() noexcept {
    BusDiagnostics<AppConfig>::scan_i2c();
}

inline void thread_send(std::string_view uri, std::string_view json_payload) noexcept {
    matter::ThreadMatterBridge<AppConfig>::send_coap_telemetry(uri, json_payload);
}

inline void ble_println(std::string_view text) noexcept {
    ble::BluetoothSerialMonitor::println(text);
}

template <typename... Args>
inline void ble_printf(const char* fmt, Args... args) noexcept {
    ble::BluetoothSerialMonitor::printf(fmt, args...);
}

inline void log_sd(std::string_view raw_line) noexcept {
    storage::SDCardLogger::write_raw_line(raw_line);
}

inline void send_sms(std::string_view phone_number, std::string_view message) noexcept {
    cellular::SMSEngine<AppConfig>::send_sms(phone_number, message);
}

inline void show_lcd(uint8_t row, std::string_view text) noexcept {
    drivers::PCF8574LCDDriver<AppConfig>::set_cursor(0, row);
    drivers::PCF8574LCDDriver<AppConfig>::print(text);
}

inline size_t cache_pending_count() noexcept {
    return cache::TieredCacheEngine<AppConfig>::pending_count();
}

inline void flush_offline_cache(size_t max_batch = 50) noexcept {
    cache::TieredCacheEngine<AppConfig>::flush_backlog_to_server(max_batch);
}

inline void clear_offline_cache() noexcept {
    cache::TieredCacheEngine<AppConfig>::clear();
}

namespace CSVProfile {
    inline constexpr auto GENERIC_INDUSTRIAL   = storage::CSVProfile::GENERIC_INDUSTRIAL;
    inline constexpr auto ENVIRONMENTAL        = storage::CSVProfile::ENVIRONMENTAL;
    inline constexpr auto ELECTRICAL_ENERGY    = storage::CSVProfile::ELECTRICAL_ENERGY;
    inline constexpr auto WATER_AQUACULTURE    = storage::CSVProfile::WATER_AQUACULTURE;
    inline constexpr auto PRECISION_AGRI       = storage::CSVProfile::PRECISION_AGRI;
    inline constexpr auto VIBRATION_STRUCTURAL = storage::CSVProfile::VIBRATION_STRUCTURAL;
    inline constexpr auto CUSTOM               = storage::CSVProfile::CUSTOM;
}

inline void csv_preset(storage::CSVProfile profile) noexcept {
    storage::SDCardLogger::set_profile(profile);
}

inline void csv_custom_header(std::string_view header) noexcept {
    storage::SDCardLogger::set_custom_header(header);
}

inline void csv_custom_formatter(storage::CSVFormatterFn fn) noexcept {
    storage::SDCardLogger::set_custom_formatter(fn);
}

inline void web_widget(std::string_view key, std::string_view label, std::string_view unit) noexcept {
    web::DashboardRegistry::add_widget(key, label, unit);
}

[[nodiscard]] inline constexpr std::string_view target_chip_name() noexcept {
    return target::TargetDetector::chip_name();
}

// ============================================================================
// 6. FLUENT SAFETY GUARDS & RULES
// ============================================================================

inline guards::RangeGuard& guard(std::string_view name) noexcept {
    return guards::GuardManager::create(name);
}

inline automation::RuleBuilder& rule(std::string_view name) noexcept {
    return automation::RulesEngine::create_rule(name);
}

inline automation::ScheduleBuilder& schedule(std::string_view name) noexcept {
    return automation::JobScheduler::create_schedule(name);
}

inline void set_encryption(security::EncryptionScheme scheme, std::string_view secret_key) noexcept {
    security::PayloadSecurityEngine::configure(scheme, secret_key);
}

inline void enable_anti_replay(bool enable, uint32_t max_skew_seconds = 60) noexcept {
    security::AntiReplayEngine::configure(enable, max_skew_seconds);
}

[[nodiscard]] inline bool is_anti_replay_enabled() noexcept {
    return security::AntiReplayEngine::is_enabled();
}

inline void check_cloud_ota(std::string_view url = "https://ota.aetheriot.io/firmware/update.bin") noexcept {
    ota::OTAManager<AppConfig>::check_and_update_from_cloud(url);
}

[[nodiscard]] inline terminal::CommandBuilder& cli(std::string_view command_name) noexcept {
    return terminal::Registry::register_command(command_name);
}

inline bool dispatch_cli(std::string_view command_line) noexcept {
    return terminal::Registry::dispatch(command_line);
}

// Environmental & Advanced Sensor Wrappers
inline sensors::BME680Data read_bme680(uint8_t i2c_addr = 0x77) noexcept {
    return sensors::BME680Driver::read(i2c_addr);
}

inline sensors::SCDData read_co2_scd(uint8_t i2c_addr = 0x62) noexcept {
    return sensors::SCDDriver::read(i2c_addr);
}

inline float read_pt100(uint8_t cs_pin, float r_nominal = 100.0f) noexcept {
    return sensors::MAX31865Driver::read_temperature_c(cs_pin, r_nominal);
}

inline float read_ultrasonic_distance_mm(int trig_pin, int echo_pin, float air_temp_c = 25.0f) noexcept {
    return sensors::UltrasonicLevelDriver::read_distance_mm(trig_pin, echo_pin, air_temp_c);
}

inline float read_sct013_current(int adc_pin, float calib = 30.0f) noexcept {
    return sensors::CurrentTransformerDriver::read_rms_current_amps(adc_pin, calib);
}

// Stepper Motor Wrappers
inline void stepper_config(int step_pin, int dir_pin, int en_pin = -1, float steps_per_rev = 200.0f) noexcept {
    stepper::StepperController::configure(step_pin, dir_pin, en_pin, steps_per_rev);
}

inline void stepper_rotate(float degrees, bool clockwise = true) noexcept {
    stepper::StepperController::rotate_degrees(degrees, clockwise);
}

inline void stepper_rpm(float rpm) noexcept {
    stepper::StepperController::set_speed_rpm(rpm);
}

// Industrial Analog Loop (4-20mA & 0-10V)
inline void set_loop_4_20ma(uint8_t channel, float current_ma) noexcept {
    analog::IndustrialLoopTransmitter::set_current_4_20ma(channel, current_ma);
}

inline void set_loop_0_10v(uint8_t channel, float voltage_v) noexcept {
    analog::IndustrialLoopTransmitter::set_voltage_0_10v(channel, voltage_v);
}

// LCD Dynamic Display Wrappers
inline display::PageBuilder& lcd_page(std::string_view title = "") noexcept {
    return display::CustomDisplayManager::page(title);
}

inline void lcd_clear_pages() noexcept {
    display::CustomDisplayManager::clear_pages();
}

inline void lcd_set_interval(uint32_t ms) noexcept {
    display::CustomDisplayManager::set_page_rotation_ms(ms);
}

inline void lcd_print(uint8_t col, uint8_t row, std::string_view text) noexcept {
    display::CustomDisplayManager::print_direct<AppConfig>(col, row, text);
}

inline void lcd_clear() noexcept {
    display::CustomDisplayManager::clear_screen<AppConfig>();
}

// Flash Partition & Memory Geometry Helpers
inline void print_partitions() noexcept {
    partitions::PartitionManager::print_memory_map();
}

[[nodiscard]] inline uint32_t flash_size_mb() noexcept {
    return partitions::PartitionManager::flash_chip_size_mb();
}

[[nodiscard]] inline std::string_view running_partition() noexcept {
    return partitions::PartitionManager::running_app_label();
}

[[nodiscard]] inline std::string_view next_ota_partition() noexcept {
    return partitions::PartitionManager::next_ota_update_label();
}

// Status LED Indicator Helpers
inline void status_led(int8_t pin, bool active_high = true) noexcept {
    indicator::StatusLED::configure(pin, active_high);
}

inline void led_pattern(indicator::LEDPattern pattern) noexcept {
    indicator::StatusLED::set_pattern(pattern);
}

inline void led_custom_pulse(uint16_t on_ms, uint16_t off_ms) noexcept {
    indicator::StatusLED::set_custom_timing(on_ms, off_ms);
}

// MQTT Configurator Helper
[[nodiscard]] inline mq::MQTTConfigurator& mqtt() noexcept {
    return mq::MQTTManager::instance();
}

// Universal 2-Point Linear Calibration Scaling
[[nodiscard]] inline constexpr float calibrate_linear(float raw_val, float in_min, float in_max, float out_min, float out_max) noexcept {
    if (in_max == in_min) return out_min;
    return (raw_val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Hardware Push Button & Interrupt Debouncer Helper
[[nodiscard]] inline input::ButtonHandler& button(int8_t gpio_pin, bool active_low = true, bool pullup = true) noexcept {
    return input::ButtonManager::button(gpio_pin, active_low, pullup);
}

// Modbus RTU Master Query Helper
[[nodiscard]] inline modbus::MasterQueryBuilder modbus_query(uint8_t slave_id = 1) noexcept {
    return modbus::MasterEngine::query(slave_id);
}

// HTTP REST Webhook Helpers
inline bool http_post(std::string_view url, std::string_view json_body) noexcept {
    return http::WebhookClient::post_json(url, json_body);
}

inline bool webhook_telegram(std::string_view bot_token, std::string_view chat_id, std::string_view message) noexcept {
    return http::WebhookClient::send_telegram(bot_token, chat_id, message);
}

inline bool webhook_discord(std::string_view webhook_url, std::string_view message) noexcept {
    return http::WebhookClient::send_discord(webhook_url, message);
}

// ============================================================================
// 7. HIGH-LEVEL APPLICATION ENGINE (DX WRAPPER)
// ============================================================================

template <typename Config = AppConfig>
class Engine {
public:
    using DataCallback  = void(*)(const SensorData& data);
    using AlertCallback = void(*)(const char* msg);

    static Engine& instance() noexcept {
        static Engine eng;
        return eng;
    }

    Engine& on_data(DataCallback cb) noexcept {
        data_cb_ = cb;
        return *this;
    }

    Engine& on_alert(AlertCallback cb) noexcept {
        alert_cb_ = cb;
        return *this;
    }

    int run() noexcept {
        // 0. Target Silicon Log
        std::printf("\033[1;36m[TARGET] Running on %.*s\033[0m\n",
                    static_cast<int>(target_chip_name().length()), target_chip_name().data());

        // 1. Blackbox Crash Forensics & OTA Rollback Guard
        diagnostics::CrashForensics::init_and_check();
        ota::OTAManager<Config>::confirm_running_partition_valid();

        auto& core = SystemEngine<Config>::instance();
        core.init();
        core.run_diagnostics();

        // 2. Protocols, BLE, Multi-Servo, Multiplexers, Captive DNS, LoRa, SD Card, PWM Expander, Thread, Rain Gauge
        protocols::MDNSEngine<Config>::init();
        protocols::NTPEngine<Config>::init();
        web::WebDashboardServer<Config>::init();
        ble::BLEProvisioner::init();
        ble::BluetoothSerialMonitor::init();
        dns::CaptiveDNSServer::start();
        lora::LoRaTransceiver<Config>::init();
        imu::MPU6050Driver<Config>::init();
        storage::SDCardLogger::init();
        actuators::PCA9685PWMExpander<Config>::init();
        actuators::MultiServoMatrix<32>::init();
        multiplexer::OneWireMultiProbeArray<16>::init();
        multiplexer::AnalogMultiplexerCD74HC4067::init();
        multiplexer::ModbusMultiSlaveHub::init();
        matter::ThreadMatterBridge<Config>::init();
        sensors::PulseRainGauge::init();

        core.start();

        while (core.is_running()) {
            core.feed_watchdog();
            
            // 3. I2C Self-Healing Check
            diagnostics::I2CSelfHealing<Config>::recover_bus_if_hung();

            core.process_telemetry();
            core.process_connectivity();
            core.process_actuation();

            const auto& raw = core.telemetry();
            const uint32_t now_ms = core.get_timestamp_ms();

            // Evaluate Cluster Node Heartbeat Timeouts
            mesh::ClusterRegistry::evaluate_timeouts(now_ms);

            // Log Telemetry to Tiered Cache & MicroSD CSV
            cache::TieredCacheEngine<Config>::push(raw);

            // Auto-update Multi-Page LCD
            display::CustomDisplayManager::update<Config>(raw, now_ms);

            // Update Status LED Indicator Non-Blocking
            indicator::StatusLED::update(now_ms);

            // Update Hardware Button Debounce & Long-Press
            input::ButtonManager::update_all(now_ms);

            // Adaptive Sampling Evaluation
            if (adaptive::AdaptiveSamplingManager::should_transmit(raw, now_ms)) {
                protocols::AsyncWebSocketStreamer<Config>::broadcast_telemetry(raw);
            }

            // Compute Temperature Compensated pH and EC
            const float raw_ph = static_cast<float>(raw.water_ph_mili) / 1000.0f;
            const float raw_ec = static_cast<float>(raw.water_ec_us_cm);
            const float w_temp = static_cast<float>(raw.water_temp_centi_c) / 100.0f;
            const float volt_v = static_cast<float>(raw.bus_voltage_mv) / 1000.0f;

            // Read IMU 6-Axis
            const auto imu_data = imu::MPU6050Driver<Config>::read().value();

            SensorData d{};
            d.temp           = static_cast<float>(raw.air_temperature_centi_c) / 100.0f;
            d.hum            = static_cast<float>(raw.air_humidity_centi_rh) / 100.0f;
            d.lux            = static_cast<float>(raw.ambient_light_lux);
            d.rain_rate_mm_h = sensors::PulseRainGauge::rainfall_rate_mm_per_hour();
            d.rain_daily_mm  = sensors::PulseRainGauge::daily_rainfall_mm();
            d.ph             = automation::TemperatureCompensation::compensate_ph(raw_ph, w_temp);
            d.ec             = automation::TemperatureCompensation::compensate_ec(raw_ec, w_temp);
            d.tds            = static_cast<float>(raw.water_tds_ppm);
            d.oxygen         = static_cast<float>(raw.dissolved_oxygen_mili) / 1000.0f;
            d.distance       = static_cast<float>(raw.water_distance_mm);
            d.volume         = static_cast<float>(raw.water_volume_liters);
            d.flow_lpm       = actuators::FlowMeterAccumulator::flow_rate_lpm();
            d.total_flow     = actuators::FlowMeterAccumulator::total_liters();
            d.tilt_pitch     = imu_data.pitch_deg;
            d.tilt_roll      = imu_data.roll_deg;
            d.vibration_g    = imu_data.vibration_g;
            d.tank_pct       = raw.tank_percentage_full;
            d.nitrogen       = raw.soil_nitrogen_mg_kg;
            d.phosphorus     = raw.soil_phosphorus_mg_kg;
            d.potassium      = raw.soil_potassium_mg_kg;
            d.voltage        = volt_v;
            d.current        = static_cast<float>(raw.bus_current_ma) / 1000.0f;
            d.power          = static_cast<float>(raw.bus_power_mw) / 1000.0f;
            d.battery_pct    = power::BatteryFuelGauge::calculate_percentage(volt_v);
            d.latitude       = raw.latitude;
            d.longitude      = raw.longitude;
            d.satellites     = raw.gps_satellites;
            d.health_pct     = adaptive::SystemHealthAnalyzer::compute_health_score(raw);
            d.ph_ok          = raw.flags.ph_valid != 0;
            d.ec_ok          = raw.flags.ec_valid != 0;
            d.level_ok       = raw.flags.level_valid != 0;
            d.online         = raw.flags.wifi_online != 0 || raw.flags.cellular_online != 0;
            d.emergency      = raw.flags.emergency_stop != 0;

            // Evaluate Automated Logic Only in AUTO Mode
            if (control::ControlStateManager::is_auto()) {
                guards::GuardManager::check("pH", d.ph);
                guards::GuardManager::check("Temperature", d.temp);
                guards::GuardManager::check("Oxygen", d.oxygen);
                guards::GuardManager::check("Tank", d.volume);
                guards::GuardManager::check("Tilt", d.tilt_pitch);

                automation::SensorSnapshot snap{
                    .temp = d.temp, .hum = d.hum, .ph = d.ph, .ec = d.ec,
                    .volume = d.volume, .oxygen = d.oxygen, .tank_pct = d.tank_pct, .online = d.online
                };
                automation::RulesEngine::evaluate_all(snap);
            }

            if (data_cb_) {
                data_cb_(d);
            }

            if (raw.flags.emergency_stop && alert_cb_) {
                alert_cb_("EMERGENCY_STOP_TRIGGERED");
            }

            core.yield(Config::Timing::LOOP_INTERVAL_MS);
        }
        return 0;
    }

private:
    Engine() noexcept = default;
    DataCallback  data_cb_{nullptr};
    AlertCallback alert_cb_{nullptr};
};

} // namespace iot

// ============================================================================
// 8. SIGNATURE FRAMEWORK MACROS
// ============================================================================

#if defined(ESP_PLATFORM)
#define IOT_APP(...) \
    void _iot_user_setup(iot::Engine<iot::AppConfig>& app); \
    extern "C" void app_main() { \
        auto& app = iot::Engine<iot::AppConfig>::instance(); \
        _iot_user_setup(app); \
        app.run(); \
    } \
    void _iot_user_setup(iot::Engine<iot::AppConfig>& __VA_ARGS__)
#else
#define IOT_APP(...) \
    void _iot_user_setup(iot::Engine<iot::AppConfig>& app); \
    int main() { \
        auto& app = iot::Engine<iot::AppConfig>::instance(); \
        _iot_user_setup(app); \
        return app.run(); \
    } \
    void _iot_user_setup(iot::Engine<iot::AppConfig>& __VA_ARGS__)
#endif
