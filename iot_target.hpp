#pragma once

/**
 * ============================================================================
 * ESPRESSIF SILICON TARGET CLASSIFIER & CAPABILITY MATRIX (iot_target.hpp)
 * ============================================================================
 * Officially Tested & Validated Espressif Silicon Targets:
 * - ESP32 Classic (Xtensa Dual-Core LX6) -> Industrial Gateways, Hydroponics
 * - ESP32-S Series (S2 Single-Core LX7, S3 Dual-Core LX7 + AI/Vector/USB)
 * - ESP32-C Series (C2 120MHz, C3 160MHz RISC-V, C6 Wi-Fi 6 + Thread/Zigbee)
 * - ESP32-H Series (H2 RISC-V 802.15.4 Thread/Zigbee/Matter + BLE 5.3)
 * - ESP32-P Series (P4 Dual-Core RISC-V High-Performance SoC)
 * - ESP8266 (Legacy Low-Cost Remote Sensor Nodes)
 * - Host Native (Linux, macOS, Windows for Unit Testing & CI/CD Simulation)
 * ============================================================================
 */

#include <cstdint>
#include <string_view>

namespace iot::target {

enum class ChipFamily : uint8_t {
    ESP32_CLASSIC,
    ESP32_S2,
    ESP32_S3,
    ESP32_C2,
    ESP32_C3,
    ESP32_C6,
    ESP32_H2,
    ESP32_P4,
    ESP8266,
    HOST_SIMULATION
};

enum class Architecture : uint8_t {
    XTENSA_LX6,
    XTENSA_LX7,
    RISC_V_32,
    X86_64_HOST,
    ARM_HOST
};

struct TargetCapabilities {
    ChipFamily   family{ChipFamily::HOST_SIMULATION};
    Architecture arch{Architecture::X86_64_HOST};
    bool         has_wifi{true};
    bool         has_ble{true};
    bool         has_ieee802154_thread_zigbee{false};
    bool         has_hardware_crypto{true};
    bool         has_usb_serial_jtag{false};
    bool         has_ulp_coprocessor{true};
    uint8_t      core_count{2};
};

class TargetDetector {
public:
    [[nodiscard]] static constexpr TargetCapabilities get_capabilities() noexcept {
#if defined(CONFIG_IDF_TARGET_ESP32)
        return TargetCapabilities{
            .family = ChipFamily::ESP32_CLASSIC,
            .arch = Architecture::XTENSA_LX6,
            .has_wifi = true,
            .has_ble = true,
            .has_ieee802154_thread_zigbee = false,
            .has_hardware_crypto = true,
            .has_usb_serial_jtag = false,
            .has_ulp_coprocessor = true,
            .core_count = 2
        };
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
        return TargetCapabilities{
            .family = ChipFamily::ESP32_S3,
            .arch = Architecture::XTENSA_LX7,
            .has_wifi = true,
            .has_ble = true,
            .has_ieee802154_thread_zigbee = false,
            .has_hardware_crypto = true,
            .has_usb_serial_jtag = true,
            .has_ulp_coprocessor = true,
            .core_count = 2
        };
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
        return TargetCapabilities{
            .family = ChipFamily::ESP32_S2,
            .arch = Architecture::XTENSA_LX7,
            .has_wifi = true,
            .has_ble = false,
            .has_ieee802154_thread_zigbee = false,
            .has_hardware_crypto = true,
            .has_usb_serial_jtag = true,
            .has_ulp_coprocessor = true,
            .core_count = 1
        };
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
        return TargetCapabilities{
            .family = ChipFamily::ESP32_C3,
            .arch = Architecture::RISC_V_32,
            .has_wifi = true,
            .has_ble = true,
            .has_ieee802154_thread_zigbee = false,
            .has_hardware_crypto = true,
            .has_usb_serial_jtag = true,
            .has_ulp_coprocessor = false,
            .core_count = 1
        };
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
        return TargetCapabilities{
            .family = ChipFamily::ESP32_C6,
            .arch = Architecture::RISC_V_32,
            .has_wifi = true, // Wi-Fi 6
            .has_ble = true,  // BLE 5.0
            .has_ieee802154_thread_zigbee = true, // Thread & Zigbee Matter
            .has_hardware_crypto = true,
            .has_usb_serial_jtag = true,
            .has_ulp_coprocessor = true,
            .core_count = 1
        };
#elif defined(CONFIG_IDF_TARGET_ESP32H2)
        return TargetCapabilities{
            .family = ChipFamily::ESP32_H2,
            .arch = Architecture::RISC_V_32,
            .has_wifi = false,
            .has_ble = true,  // BLE 5.3
            .has_ieee802154_thread_zigbee = true, // Thread & Zigbee
            .has_hardware_crypto = true,
            .has_usb_serial_jtag = true,
            .has_ulp_coprocessor = false,
            .core_count = 1
        };
#elif defined(CONFIG_IDF_TARGET_ESP32P4)
        return TargetCapabilities{
            .family = ChipFamily::ESP32_P4,
            .arch = Architecture::RISC_V_32,
            .has_wifi = false,
            .has_ble = false,
            .has_ieee802154_thread_zigbee = false,
            .has_hardware_crypto = true,
            .has_usb_serial_jtag = true,
            .has_ulp_coprocessor = true,
            .core_count = 2
        };
#elif defined(ESP8266)
        return TargetCapabilities{
            .family = ChipFamily::ESP8266,
            .arch = Architecture::XTENSA_LX6,
            .has_wifi = true,
            .has_ble = false,
            .has_ieee802154_thread_zigbee = false,
            .has_hardware_crypto = false,
            .has_usb_serial_jtag = false,
            .has_ulp_coprocessor = false,
            .core_count = 1
        };
#else
        return TargetCapabilities{
            .family = ChipFamily::HOST_SIMULATION,
            .arch = Architecture::X86_64_HOST,
            .has_wifi = true,
            .has_ble = true,
            .has_ieee802154_thread_zigbee = true,
            .has_hardware_crypto = true,
            .has_usb_serial_jtag = true,
            .has_ulp_coprocessor = true,
            .core_count = 8
        };
#endif
    }

    [[nodiscard]] static constexpr std::string_view chip_name() noexcept {
        constexpr auto caps = get_capabilities();
        switch (caps.family) {
            case ChipFamily::ESP32_CLASSIC:  return "ESP32 Classic (Dual-Core Xtensa LX6)";
            case ChipFamily::ESP32_S2:       return "ESP32-S2 (Single-Core Xtensa LX7)";
            case ChipFamily::ESP32_S3:       return "ESP32-S3 (Dual-Core Xtensa LX7 + Vector AI)";
            case ChipFamily::ESP32_C2:       return "ESP32-C2 (RISC-V 120MHz)";
            case ChipFamily::ESP32_C3:       return "ESP32-C3 (RISC-V 160MHz + BLE 5)";
            case ChipFamily::ESP32_C6:       return "ESP32-C6 (RISC-V Wi-Fi 6 + Thread/Zigbee)";
            case ChipFamily::ESP32_H2:       return "ESP32-H2 (RISC-V Thread/Zigbee/Matter)";
            case ChipFamily::ESP32_P4:       return "ESP32-P4 (High-Performance Dual RISC-V)";
            case ChipFamily::ESP8266:        return "ESP8266 (Xtensa L106)";
            default:                         return "Host Native Simulation Environment";
        }
    }
};

} // namespace iot::target
