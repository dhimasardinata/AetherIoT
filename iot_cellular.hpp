#pragma once

/**
 * ============================================================================
 * CELLULAR SMS REMOTE COMMAND & ALERT ENGINE (iot_cellular.hpp)
 * ============================================================================
 * Handles incoming SMS command execution and outgoing SMS alert broadcasts
 * over SIM7600 (4G LTE) / SIM800L (2G) modems with zero heap allocations.
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <string_view>
#include <span>
#include <array>

#include "config.hpp"
#include "iot_core.hpp"

namespace iot::cellular {

template <typename Config>
class SMSEngine {
public:
    static Result<void> send_sms(std::string_view phone_number, std::string_view message) noexcept {
        if constexpr (!Config::Features::ENABLE_CELLULAR) return Status::OK;

        std::printf("\033[1;36m[SMS-OUT] Sending SMS to %.*s: %.*s\033[0m\n",
                    static_cast<int>(phone_number.length()), phone_number.data(),
                    static_cast<int>(message.length()), message.data());

#if defined(ESP_PLATFORM)
        const uart_port_t port = static_cast<uart_port_t>(Config::Bus::MODEM_UART_PORT);
        
        // AT+CMGF=1 (Text Mode)
        uart_write_bytes(port, "AT+CMGF=1\r\n", 11);
        vTaskDelay(pdMS_TO_TICKS(100));

        // AT+CMGS="phoneNumber"
        FixedString<64> cmgs_cmd;
        cmgs_cmd.format("AT+CMGS=\"%.*s\"\r\n", static_cast<int>(phone_number.length()), phone_number.data());
        uart_write_bytes(port, cmgs_cmd.c_str(), cmgs_cmd.length());
        vTaskDelay(pdMS_TO_TICKS(100));

        // Message Body + Ctrl-Z (0x1A)
        uart_write_bytes(port, message.data(), message.size());
        const uint8_t ctrl_z = 0x1A;
        uart_write_bytes(port, &ctrl_z, 1);
#endif
        return Status::OK;
    }

    static void parse_inbound_sms(std::string_view sender, std::string_view text, const UnifiedTelemetry& current_data) noexcept {
        std::printf("\033[1;32m[SMS-IN] Received from %.*s: %.*s\033[0m\n",
                    static_cast<int>(sender.length()), sender.data(),
                    static_cast<int>(text.length()), text.data());

        if (text == "STATUS" || text == "status") {
            FixedString<160> reply;
            reply.format("STATUS: pH=%.2f EC=%ld TDS=%ld Temp=%.1fC Hum=%.1f%% Vol=%luL(%u%%)",
                         static_cast<float>(current_data.water_ph_mili) / 1000.0f,
                         static_cast<long>(current_data.water_ec_us_cm),
                         static_cast<long>(current_data.water_tds_ppm),
                         static_cast<float>(current_data.air_temperature_centi_c) / 100.0f,
                         static_cast<float>(current_data.air_humidity_centi_rh) / 100.0f,
                         static_cast<unsigned long>(current_data.water_volume_liters),
                         current_data.tank_percentage_full);
            send_sms(sender, reply.string_view());
        } else if (text == "PUMP ON 1") {
            ActuatorEngine<Config>::set_relay(1, true);
            send_sms(sender, "OK: Pompa 1 dinyalakan");
        } else if (text == "PUMP OFF 1") {
            ActuatorEngine<Config>::set_relay(1, false);
            send_sms(sender, "OK: Pompa 1 dimatikan");
        } else if (text == "STOP") {
            ActuatorEngine<Config>::set_relay(1, false);
            ActuatorEngine<Config>::set_relay(2, false);
            ActuatorEngine<Config>::set_relay(3, false);
            ActuatorEngine<Config>::set_relay(4, false);
            send_sms(sender, "OK: EMERGENCY STOP DIAKTIFKAN");
        }
    }
};

} // namespace iot::cellular
