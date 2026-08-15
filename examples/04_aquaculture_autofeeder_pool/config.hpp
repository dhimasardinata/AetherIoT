#pragma once

#include <cstdint>
#include <string_view>

namespace iot {

struct LocalAppConfig {
    struct System {
        static constexpr std::string_view DEVICE_NAME   = "Aether-Aquaculture";
        static constexpr std::string_view FIRMWARE_VER  = "0.0.1";
        static constexpr std::string_view HARDWARE_REV  = "AQUA-PRO-1.0";
    };

    struct Connectivity {
        static constexpr std::string_view DEFAULT_SSID  = "AquaFarm_Pool_WiFi";
        static constexpr std::string_view DEFAULT_PASS  = "AquaPass2026";
        static constexpr std::string_view AP_FALLBACK_SSID = "AETHER-AQUA-AP";
        static constexpr std::string_view AP_FALLBACK_PASS = "12345678";
    };

    struct Timing {
        static constexpr uint32_t LOOP_INTERVAL_MS      = 1000;
        static constexpr uint32_t TELEMETRY_INTERVAL_MS = 2000;
        static constexpr uint32_t DISPLAY_PAGE_TIME_MS  = 3000;
    };

    struct Storage {
        static constexpr size_t FLASH_LOG_SECTOR_SIZE   = 4096;
    };

    struct Bus {
        static constexpr uint32_t I2C_FREQ_HZ           = 400000;
        static constexpr uint32_t MODBUS_BAUD           = 9600;
    };

    struct Sensors {
        struct I2CDevices {
            static constexpr bool ENABLE_LCD_PCF8574    = true;
        };
    };

    struct Actuators {
        static constexpr int8_t CHANNEL_PINS[16] = {
            25, 26, 27, 33, 13, 14, 15, 2, 4, 0, -1, -1, -1, -1, -1, -1
        };
        static constexpr bool ACTIVE_POLARITY_HIGH = false;

        enum Alias : uint8_t {
            AERATOR      = 8,   // Pond Aerator Oxygenator
            AUTO_FEEDER  = 9    // Dispenser Feeder
        };
    };
};

} // namespace iot
