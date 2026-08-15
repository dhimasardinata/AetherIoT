#pragma once

#include <cstdint>
#include <string_view>
#include <array>

namespace iot {

/**
 * ============================================================================
 * UNIFIED DECLARATIVE IOT SYSTEM CONFIGURATION BLUEPRINT
 * ============================================================================
 * Single-Point of Truth: All hardware mapping, 16-channel actuator aliases,
 * bus timings, sensor registries, mDNS, NTP, WebSocket, and local encryption.
 * ============================================================================
 */
struct AppConfig {
    // ------------------------------------------------------------------------
    // 1. FIRMWARE IDENTITY & LOCAL mDNS
    // ------------------------------------------------------------------------
    struct System {
        static constexpr std::string_view DEVICE_NAME     = "UNIFIED-ENTERPRISE-MASTER";
        static constexpr std::string_view MDNS_HOSTNAME   = "aether-node"; // Access via http://aether-node.local
        static constexpr std::string_view AP_SSID       = "AETHER-PORTAL";
        static constexpr std::string_view TELEMETRY_URL = "https://api.aetheriot.io/api/v1/telemetry";
        static constexpr std::string_view HARDWARE_REV    = "ESP32-PRO-INDUSTRIAL-V5";
        static constexpr std::string_view FIRMWARE_VER    = "5.0.0-ENTERPRISE";
        static constexpr uint32_t         DEBUG_BAUD_RATE = 115200;
        static constexpr bool             ENABLE_ANSI_LOG = true;
    };

    // ------------------------------------------------------------------------
    // 2. 16-CHANNEL ACTUATORS & DESCRIPTIVE NAMED ALIASES
    // ------------------------------------------------------------------------
    struct Actuators {
        static constexpr size_t NUM_CHANNELS = 16;

        // Direct GPIO Pin Mapping for 16 Channels (-1 if handled via I2C/Modbus expander)
        static constexpr std::array<int8_t, 16> GPIO_PINS = {
            25, 26, 27, 33,  // Ch 1-4  (Pumps / Solenoids)
            13, 14, 15, 2,   // Ch 5-8  (Grow Light, Exhaust Fan, Heater, Aerator)
            4,  0,  -1, -1,  // Ch 9-12 (Feeder, Drain Valve, Misting, UV Sterile)
            -1, -1, -1, -1   // Ch 13-16(Buzzer, Aux 1, Aux 2, Aux 3)
        };

        // Active State Polarity (false = Active LOW for typical relay boards, true = Active HIGH)
        static constexpr bool ACTIVE_POLARITY_HIGH = false;

        // Default Compile-Time Generic Industrial Aliases
        enum Alias : uint8_t {
            CH_1         = 1,   // Actuator Channel 1
            CH_2         = 2,   // Actuator Channel 2
            CH_3         = 3,   // Actuator Channel 3
            CH_4         = 4,   // Actuator Channel 4
            CH_5         = 5,   // Actuator Channel 5
            CH_6         = 6,   // Actuator Channel 6
            CH_7         = 7,   // Actuator Channel 7
            CH_8         = 8,   // Actuator Channel 8
            VALVE_MAIN   = 9,   // Primary Control Valve / Solenoid
            PUMP_MAIN    = 10,  // Primary Circulation / Process Pump
            FAN_EXHAUST  = 11,  // Exhaust / Cooling Fan
            HEATER_MAIN  = 12,  // Main Heating Element
            COOLER_MAIN  = 13,  // Cooling Unit / Chiller
            MOTOR_DRIVE  = 14,  // Drive Motor / Stepper Actuator
            ALARM_BUZZER = 15,  // Warning Siren / Alarm Buzzer
            STATUS_LAMP  = 16   // Enclosure Status Indicator Lamp
        };
    };

    // ------------------------------------------------------------------------
    // 3. HARDWARE PIN DEFINITIONS & BUS ASSIGNMENTS
    // ------------------------------------------------------------------------
    struct Pins {
        // I2C Master Bus
        static constexpr int8_t I2C_SDA = 21;
        static constexpr int8_t I2C_SCL = 22;

        // RS-485 / Modbus Bus (UART2)
        static constexpr int8_t RS485_TX    = 17;
        static constexpr int8_t RS485_RX    = 16;
        static constexpr int8_t RS485_DE_RE = 4;

        // Dedicated UART Sensor Bus (UART1)
        static constexpr int8_t UART_SENSOR_TX = 1;
        static constexpr int8_t UART_SENSOR_RX = 3;

        // Cellular Modem (SIM7600 4G / SIM800 2G)
        static constexpr int8_t MODEM_TX     = 14;
        static constexpr int8_t MODEM_RX     = 12;
        static constexpr int8_t MODEM_PWRKEY = 15;
        static constexpr int8_t MODEM_RST    = 13;
        static constexpr int8_t MODEM_EN     = 32;

        // SPI Master Bus
        static constexpr int8_t SPI_SCK   = 18;
        static constexpr int8_t SPI_MISO  = 19;
        static constexpr int8_t SPI_MOSI  = 23;
        static constexpr int8_t SD_CS     = 5;
        static constexpr int8_t LORA_CS   = 2;
        static constexpr int8_t LORA_RST  = 0;
        static constexpr int8_t LORA_DIO0 = 34;

        // Load Cell HX711
        static constexpr int8_t HX711_DOUT = 35;
        static constexpr int8_t HX711_SCK  = 36;

        // Status & Button
        static constexpr int8_t STATUS_LED  = 2;
        static constexpr int8_t USER_BUTTON = 0;
    };

    // ------------------------------------------------------------------------
    // 4. BUS CONTROLLER CONFIGURATIONS
    // ------------------------------------------------------------------------
    struct Bus {
        static constexpr uint32_t I2C_FREQ_HZ          = 400000;
        static constexpr uint32_t I2C_TIMEOUT_MS       = 50;
        static constexpr uint32_t MODBUS_BAUD          = 9600;
        static constexpr uint8_t  MODBUS_UART_PORT     = 2;
        static constexpr uint32_t MODBUS_TIMEOUT_MS    = 100;
        static constexpr uint32_t UART_SENSOR_BAUD     = 9600;
        static constexpr uint8_t  UART_SENSOR_PORT     = 1;
        static constexpr uint32_t MODEM_BAUD           = 115200;
        static constexpr uint8_t  MODEM_UART_PORT      = 0;
    };

    // ------------------------------------------------------------------------
    // 5. SENSOR INVENTORY & PERIPHERAL ADDRESSING
    // ------------------------------------------------------------------------
    struct Sensors {
        struct I2CDevices {
            static constexpr bool    ENABLE_BH1750     = true;
            static constexpr uint8_t ADDR_BH1750       = 0x23;

            static constexpr bool    ENABLE_SHT3X      = true;
            static constexpr uint8_t ADDR_SHT3X        = 0x44;

            static constexpr bool    ENABLE_BME280     = true;
            static constexpr uint8_t ADDR_BME280       = 0x76;

            static constexpr bool    ENABLE_INA219     = true;
            static constexpr uint8_t ADDR_INA219       = 0x40;

            static constexpr bool    ENABLE_RTC_DS3231 = true;
            static constexpr uint8_t ADDR_RTC          = 0x68;

            static constexpr bool    ENABLE_LCD_PCF8574= true;
            static constexpr uint8_t ADDR_LCD          = 0x27;
            static constexpr uint8_t LCD_COLS          = 20;
            static constexpr uint8_t LCD_ROWS          = 4;

            static constexpr bool    ENABLE_SSD1306    = false;
            static constexpr uint8_t ADDR_SSD1306      = 0x3C;
        } i2c;

        struct ModbusSlaves {
            static constexpr bool     ENABLE_PH         = true;
            static constexpr uint8_t  SLAVE_ID_PH       = 0x01;
            static constexpr uint16_t REG_PH_VAL        = 0x0000;

            static constexpr bool     ENABLE_EC         = true;
            static constexpr uint8_t  SLAVE_ID_EC       = 0x02;
            static constexpr uint16_t REG_EC_VAL        = 0x0002;
            static constexpr uint16_t REG_EC_TEMP       = 0x0000;
            static constexpr uint16_t REG_TDS_VAL       = 0x0004;
            static constexpr uint16_t REG_SALINITY      = 0x0003;

            static constexpr bool     ENABLE_DO         = true;
            static constexpr uint8_t  SLAVE_ID_DO       = 0x03;
            static constexpr uint16_t REG_DO_VAL        = 0x0000;

            static constexpr bool     ENABLE_SOIL_NPK   = true;
            static constexpr uint8_t  SLAVE_ID_SOIL     = 0x05;
            static constexpr uint16_t REG_SOIL_MOISTURE = 0x0000;
            static constexpr uint16_t REG_SOIL_TEMP     = 0x0001;
            static constexpr uint16_t REG_SOIL_EC       = 0x0002;
            static constexpr uint16_t REG_SOIL_PH       = 0x0003;
            static constexpr uint16_t REG_SOIL_N        = 0x0004;
            static constexpr uint16_t REG_SOIL_P        = 0x0005;
            static constexpr uint16_t REG_SOIL_K        = 0x0006;

            static constexpr bool     ENABLE_PZEM       = false;
            static constexpr uint8_t  SLAVE_ID_PZEM     = 0xF8;
        } modbus;

        struct UARTSensors {
            static constexpr bool ENABLE_A02YYUW_ULTRASONIC = true;
            static constexpr bool ENABLE_PMS5003_AIR        = false;
        } uart;

        struct LoadCell {
            static constexpr bool  ENABLE_HX711 = false;
            static constexpr float CALIBRATION_FACTOR = 420.5f;
        } load_cell;
    };

    // ------------------------------------------------------------------------
    // 6. TANK GEOMETRY & DOSING
    // ------------------------------------------------------------------------
    struct Tank {
        static constexpr uint32_t TOTAL_HEIGHT_MM     = 1000;
        static constexpr uint32_t SENSOR_DEAD_ZONE_MM = 200;
        static constexpr uint32_t TANK_DIAMETER_MM    = 600;
        static constexpr uint32_t TOTAL_CAPACITY_L    = 250;
    };

    struct Control {
        static constexpr uint32_t MAX_RELAY_ON_TIME_MS  = 60000;
        static constexpr uint32_t DOSING_PULSE_MS       = 2500;
        static constexpr uint32_t DOSING_INTERVAL_MS    = 30000;
        static constexpr uint32_t DAILY_MAX_DOSE_ML     = 500;
        static constexpr float    ML_PER_SECOND         = 1.2f;
        static constexpr int32_t  PH_TARGET_MILI        = 6500;
        static constexpr int32_t  PH_HYSTERESIS_MILI    = 300;
        static constexpr int32_t  EC_TARGET_US_CM       = 1800;
        static constexpr int32_t  EC_HYSTERESIS_US_CM   = 100;
    };

    struct Calibration {
        static constexpr float PH_SLOPE             = 1.000f;
        static constexpr float PH_OFFSET            = 0.000f;
        static constexpr float EC_SLOPE             = 1.000f;
        static constexpr float EC_OFFSET            = 0.000f;
        static constexpr int32_t WATER_LEVEL_OFFSET_MM = 0;
    };

    struct DSP {
        static constexpr uint8_t  MOVING_AVG_WINDOW_SIZE = 8;
        static constexpr uint16_t SENSOR_EXPIRATION_MS   = 15000;
        static constexpr float    EMA_ALPHA              = 0.25f;
    };

    // ------------------------------------------------------------------------
    // 7. NETWORK, NTP, mDNS & ASYNC WEBSOCKET PROTOCOLS
    // ------------------------------------------------------------------------
    struct Network {
        // WiFi Station
        static constexpr std::string_view WIFI_SSID     = "Gateway_Industrial_AP";
        static constexpr std::string_view WIFI_PASSWORD = "StrongSecurePassword123";
        static constexpr uint8_t          WIFI_MAX_RETRY= 5;

        // Fallback SoftAP
        static constexpr std::string_view AP_SSID       = "AETHER-PORTAL";
        static constexpr std::string_view AP_PASSWORD   = "admin12345";

        // NTP Time Synchronization (UTC+7 WIB Indonesia)
        static constexpr std::string_view NTP_SERVER_1  = "id.pool.ntp.org";
        static constexpr std::string_view NTP_SERVER_2  = "pool.ntp.org";
        static constexpr int32_t          TIMEZONE_OFFSET_SEC = 7 * 3600; // +7 Hours (WIB)

        // Web Server & Async WebSocket
        static constexpr uint16_t HTTP_SERVER_PORT      = 80;
        static constexpr uint16_t WS_STREAM_INTERVAL_MS = 100; // 10 Hz Real-Time Stream

        // Cellular APN
        static constexpr std::string_view CELLULAR_APN  = "internet";

        // Cloud Telemetry REST
        static constexpr std::string_view TELEMETRY_URL = "https://api.aetheriot.io/api/v1/telemetry";
        static constexpr std::string_view AUTH_TOKEN    = "Bearer aether_live_prod_token_sec_key";
    };

    // ------------------------------------------------------------------------
    // 8. TIME, SCHEDULER & TIMINGS
    // ------------------------------------------------------------------------
    struct Timing {
        static constexpr uint32_t LOOP_INTERVAL_MS      = 20;
        static constexpr uint32_t ACQUISITION_PERIOD_MS = 2000;
        static constexpr uint32_t TELEMETRY_PERIOD_MS   = 10000;
        static constexpr uint32_t DIAGNOSTIC_PERIOD_MS  = 30000;
        static constexpr uint32_t DISPLAY_PAGE_TIME_MS  = 3000;
        static constexpr uint32_t SD_LOG_PERIOD_MS      = 60000;
        static constexpr uint32_t WDT_TIMEOUT_MS        = 8000;
    };

    // ------------------------------------------------------------------------
    // 9. STORAGE & MEMORY SIZING
    // ------------------------------------------------------------------------
    struct Storage {
        static constexpr uint16_t TELEMETRY_RING_BUFFER_CAPACITY = 64;
        static constexpr uint16_t MODBUS_RX_BUFFER_SIZE          = 128;
        static constexpr uint16_t UART_SENSOR_RX_BUFFER_SIZE     = 64;
        static constexpr uint16_t JSON_STATIC_PAYLOAD_CAPACITY   = 512;
    };

    // ------------------------------------------------------------------------
    // 10. COMPILE-TIME FEATURE FLAGS
    // ------------------------------------------------------------------------
    struct Features {
        static constexpr bool ENABLE_WIFI              = true;
        static constexpr bool ENABLE_CELLULAR          = true;
        static constexpr bool ENABLE_LORA              = false;
        static constexpr bool ENABLE_ESP_NOW           = true;
        static constexpr bool ENABLE_MDNS              = true;
        static constexpr bool ENABLE_NTP_TIME_SYNC     = true;
        static constexpr bool ENABLE_ASYNC_WEBSOCKET   = true;
        static constexpr bool ENABLE_OTA               = true;
        static constexpr bool ENABLE_WATCHDOG          = true;
        static constexpr bool ENABLE_AUTO_DIAGNOSTICS  = true;
        static constexpr bool ENABLE_OFFLINE_CACHE     = true;
        static constexpr bool ENABLE_PRECISION_DOSING  = true;
        static constexpr bool ENABLE_SD_LOGGING        = true;
        static constexpr bool ENABLE_GNSS_GPS          = true;
    };
};

} // namespace iot
