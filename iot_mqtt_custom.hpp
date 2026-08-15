#pragma once

/**
 * ============================================================================
 * CUSTOMIZABLE MQTT BROKER, TOPIC & PAYLOAD SCHEMA (iot_mqtt_custom.hpp)
 * ============================================================================
 * Features:
 * 1. Configurable Broker Host, Port (Plain/TLS), Client ID, Auth Credentials.
 * 2. 100% User-Customizable Publish & Subscribe Topic Paths.
 * 3. Dynamic QoS Level (0, 1, 2) and Retain Flag.
 * 4. User-Defined Custom JSON Formatter Callback.
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <string_view>
#include "config.hpp"
#include "iot_core.hpp"

namespace iot::mq {

using MQTTFormatterFn = void(*)(const UnifiedTelemetry& data, FixedString<512>& out);

class MQTTConfigurator {
public:
    MQTTConfigurator& broker(std::string_view host, uint16_t port = 1883) noexcept {
        host_.assign(host);
        port_ = port;
        return *this;
    }

    MQTTConfigurator& credentials(std::string_view client_id, std::string_view user = "", std::string_view pass = "") noexcept {
        client_id_.assign(client_id);
        user_.assign(user);
        pass_.assign(pass);
        return *this;
    }

    MQTTConfigurator& telemetry_topic(std::string_view topic) noexcept {
        pub_topic_.assign(topic);
        return *this;
    }

    MQTTConfigurator& command_topic(std::string_view topic) noexcept {
        sub_topic_.assign(topic);
        return *this;
    }

    MQTTConfigurator& qos(uint8_t qos_level) noexcept {
        qos_ = (qos_level <= 2) ? qos_level : 0;
        return *this;
    }

    MQTTConfigurator& retain(bool retain_flag) noexcept {
        retain_ = retain_flag;
        return *this;
    }

    MQTTConfigurator& formatter(MQTTFormatterFn fn) noexcept {
        custom_formatter_ = fn;
        return *this;
    }

    void publish_telemetry(const UnifiedTelemetry& data) const noexcept {
        FixedString<512> payload;
        if (custom_formatter_) {
            custom_formatter_(data, payload);
        } else {
            const float temp = static_cast<float>(data.air_temperature_centi_c) / 100.0f;
            const float hum = static_cast<float>(data.air_humidity_centi_rh) / 100.0f;
            const float volt = static_cast<float>(data.bus_voltage_mv) / 1000.0f;
            payload.format("{\"temp\":%.2f,\"hum\":%.1f,\"volt\":%.2f,\"online\":true}", temp, hum, volt);
        }

        std::printf("\033[1;36m[MQTT-PUB] Topic: '%.*s' | QoS: %u | Retain: %d\033[0m\n",
                    static_cast<int>(pub_topic_.length()), pub_topic_.data(), qos_, retain_);
        std::printf("[MQTT-PAYLOAD] %.*s\n", static_cast<int>(payload.length()), payload.data());
    }

    [[nodiscard]] std::string_view get_telemetry_topic() const noexcept { return pub_topic_.string_view(); }
    [[nodiscard]] std::string_view get_command_topic() const noexcept { return sub_topic_.string_view(); }
    [[nodiscard]] uint16_t get_port() const noexcept { return port_; }

private:
    FixedString<64> host_{"broker.hivemq.com"};
    uint16_t port_{1883};
    FixedString<32> client_id_{"AetherNode-01"};
    FixedString<32> user_{};
    FixedString<32> pass_{};
    FixedString<64> pub_topic_{"aether/telemetry"};
    FixedString<64> sub_topic_{"aether/cmd"};
    uint8_t qos_{0};
    bool retain_{false};
    MQTTFormatterFn custom_formatter_{nullptr};
};

class MQTTManager {
public:
    static MQTTConfigurator& instance() noexcept {
        return config_;
    }

private:
    static inline MQTTConfigurator config_{};
};

} // namespace iot::mq
