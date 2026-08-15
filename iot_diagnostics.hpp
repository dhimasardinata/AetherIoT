#pragma once

/**
 * ============================================================================
 * BLACKBOX CRASH RECORDER & HARDWARE SELF-HEALING (iot_diagnostics.hpp)
 * ============================================================================
 * - Strongly-Typed Diagnostic Return Enums (BootReason, I2CRecoveryStatus, SystemHealthGrade)
 * - RTC Fast Memory Crash & Brownout Forensics (Survives panics and cold boots)
 * - Hardware I2C Bus Autoclear & Clock-Stretching Unwedge Engine (9-Clock Recovery)
 * - Automatic Hardware Reset Reason Analyzer & Anomaly Counter
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string_view>
#include <span>
#include <array>

#include "config.hpp"
#include "iot_core.hpp"

#if defined(ESP_PLATFORM)
#include "esp_system.h"
#include "rom/rtc.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#endif

namespace iot::diagnostics {

// ============================================================================
// 1. STRONGLY-TYPED DIAGNOSTIC RETURN ENUMS
// ============================================================================

enum class BootReason : uint8_t {
    POWER_ON_RESET     = 0,
    SOFTWARE_RESET     = 1,
    DEEP_SLEEP_WAKE    = 2,
    TASK_WDT_RESET     = 3,
    INTERRUPT_WDT      = 4,
    PANIC_CRASH        = 5,
    BROWNOUT_RESET     = 6,
    SDIO_RESET         = 7,
    UNKNOWN            = 255
};

enum class I2CRecoveryStatus : uint8_t {
    BUS_OK             = 0,
    BUS_RECOVERED      = 1,
    RECOVERY_FAILED    = 2,
    HARDWARE_FAULT     = 3
};

enum class SystemHealthGrade : uint8_t {
    OPTIMAL            = 0, // 90-100% Health
    DEGRADED_WARNING   = 1, // 60-89% Health
    CRITICAL_FAULT     = 2, // < 60% Health
    EMERGENCY_LOCKOUT  = 3
};

// ============================================================================
// 2. RTC FAST MEMORY BLACKBOX CRASH FORENSICS
// ============================================================================

struct CrashLogRecord {
    uint32_t magic{0x424C4143}; // "BLAC"
    uint32_t reset_reason{0};
    uint32_t uptime_before_crash_s{0};
    uint32_t free_heap_before_crash{0};
    uint32_t crash_count{0};
    uint32_t brownout_count{0};
    uint32_t wdt_count{0};
    char     last_fault_tag[32]{"NONE"};
};

#if defined(ESP_PLATFORM)
RTC_NOINIT_ATTR static CrashLogRecord rtc_crash_log;
#else
static CrashLogRecord rtc_crash_log;
#endif

class CrashForensics {
public:
    static BootReason init_and_check() noexcept {
        BootReason result = BootReason::POWER_ON_RESET;
#if defined(ESP_PLATFORM)
        esp_reset_reason_t reason = esp_reset_reason();

        if (rtc_crash_log.magic != 0x424C4143) {
            rtc_crash_log.magic = 0x424C4143;
            rtc_crash_log.crash_count = 0;
            rtc_crash_log.brownout_count = 0;
            rtc_crash_log.wdt_count = 0;
        }

        rtc_crash_log.reset_reason = static_cast<uint32_t>(reason);

        switch (reason) {
            case ESP_RST_POWERON:
                result = BootReason::POWER_ON_RESET;
                std::printf("\033[1;32m[BLACKBOX] Normal System Boot (Clean Power-On)\033[0m\n");
                break;
            case ESP_RST_SW:
                result = BootReason::SOFTWARE_RESET;
                break;
            case ESP_RST_DEEPSLEEP:
                result = BootReason::DEEP_SLEEP_WAKE;
                break;
            case ESP_RST_PANIC:
                result = BootReason::PANIC_CRASH;
                rtc_crash_log.crash_count++;
                std::printf("\033[1;31m[BLACKBOX] SYSTEM RECOVERED FROM PANIC CRASH (Count: %lu)\033[0m\n",
                            static_cast<unsigned long>(rtc_crash_log.crash_count));
                break;
            case ESP_RST_INT_WDT:
                result = BootReason::INTERRUPT_WDT;
                rtc_crash_log.wdt_count++;
                break;
            case ESP_RST_TASK_WDT:
                result = BootReason::TASK_WDT_RESET;
                rtc_crash_log.wdt_count++;
                std::printf("\033[1;31m[BLACKBOX] SYSTEM RECOVERED FROM TASK WDT RESET (Count: %lu)\033[0m\n",
                            static_cast<unsigned long>(rtc_crash_log.wdt_count));
                break;
            case ESP_RST_BROWNOUT:
                result = BootReason::BROWNOUT_RESET;
                rtc_crash_log.brownout_count++;
                std::printf("\033[1;31m[BLACKBOX] BROWNOUT VOLTAGE DROP DETECTED (Count: %lu)\033[0m\n",
                            static_cast<unsigned long>(rtc_crash_log.brownout_count));
                break;
            default:
                result = BootReason::UNKNOWN;
                break;
        }
#endif
        return result;
    }

    static void record_fault(std::string_view tag, uint32_t uptime_s, uint32_t heap_bytes) noexcept {
        rtc_crash_log.uptime_before_crash_s = uptime_s;
        rtc_crash_log.free_heap_before_crash = heap_bytes;
        const size_t len = (tag.size() < 31) ? tag.size() : 31;
        std::memcpy(rtc_crash_log.last_fault_tag, tag.data(), len);
        rtc_crash_log.last_fault_tag[len] = '\0';
    }

    [[nodiscard]] static const CrashLogRecord& get_record() noexcept {
        return rtc_crash_log;
    }
};

// ============================================================================
// 3. HARDWARE I2C BUS AUTOCLEAR & UNWEDGE ENGINE
// ============================================================================

template <typename Config>
class I2CSelfHealing {
public:
    static I2CRecoveryStatus recover_bus_if_hung() noexcept {
#if defined(ESP_PLATFORM)
        const auto sda_pin = static_cast<gpio_num_t>(Config::Pins::I2C_SDA);
        const auto scl_pin = static_cast<gpio_num_t>(Config::Pins::I2C_SCL);

        // Configure pins as open-drain GPIO to check if SDA is held LOW by a frozen slave
        gpio_set_direction(sda_pin, GPIO_MODE_INPUT);
        gpio_set_direction(scl_pin, GPIO_MODE_OUTPUT);

        if (gpio_get_level(sda_pin) == 0) {
            std::printf("\033[1;33m[I2C-HEAL] I2C Bus Hang Detected (SDA stuck LOW). Applying 9 SCL recovery clock pulses...\033[0m\n");

            // Clock SCL up to 9 times until slave releases SDA
            bool released = false;
            for (int i = 0; i < 9; ++i) {
                gpio_set_level(scl_pin, 0);
                esp_rom_delay_us(5);
                gpio_set_level(scl_pin, 1);
                esp_rom_delay_us(5);
                if (gpio_get_level(sda_pin) == 1) {
                    released = true;
                    break;
                }
            }

            // Generate STOP condition: SDA goes HIGH while SCL is HIGH
            gpio_set_direction(sda_pin, GPIO_MODE_OUTPUT);
            gpio_set_level(sda_pin, 0);
            esp_rom_delay_us(5);
            gpio_set_level(scl_pin, 1);
            esp_rom_delay_us(5);
            gpio_set_level(sda_pin, 1);
            esp_rom_delay_us(5);

            if (released) {
                std::printf("\033[1;32m[I2C-HEAL] I2C Bus Successfully Recovered & Re-initialized.\033[0m\n");
                i2c_driver_delete(I2C_NUM_0);
                I2CBusMaster<Config>::init();
                return I2CRecoveryStatus::BUS_RECOVERED;
            } else {
                std::printf("\033[1;31m[I2C-HEAL] I2C Bus Recovery Failed! Hardware Fault.\033[0m\n");
                return I2CRecoveryStatus::RECOVERY_FAILED;
            }
        }
        return I2CRecoveryStatus::BUS_OK;
#else
        return I2CRecoveryStatus::BUS_OK;
#endif
    }
};

} // namespace iot::diagnostics
