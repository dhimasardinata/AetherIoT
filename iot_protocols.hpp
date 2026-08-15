#pragma once

/**
 * ============================================================================
 * NETWORK PROTOCOLS, WSS & ENCRYPTED WEBSOCKET (iot_protocols.hpp)
 * ============================================================================
 * - mDNS Auto-Discovery (http://<hostname>.local)
 * - SNTP Network Time Synchronization (UTC+7 WIB Indonesia)
 * - Async WebSocket Live Telemetry Streaming with Multi-Scheme Payload Encryption
 * - Local Security & Token Authenticator
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <string_view>
#include <span>
#include <array>

#include "config.hpp"
#include "iot_core.hpp"
#include "iot_crypto.hpp"
#include "iot_security.hpp"

#if defined(ESP_PLATFORM)
#include "mdns.h"
#include "esp_sntp.h"
#endif

namespace iot::protocols {

// ============================================================================
// 1. mDNS LOCAL HOSTNAME AUTO-DISCOVERY
// ============================================================================

template <typename Config>
class MDNSEngine {
public:
    static Result<void> init() noexcept {
        if constexpr (!Config::Features::ENABLE_MDNS) return Status::OK;
#if defined(ESP_PLATFORM)
        esp_err_t err = mdns_init();
        if (err != ESP_OK) return Status::ERROR_HARDWARE_FAIL;

        mdns_hostname_set(Config::System::MDNS_HOSTNAME.data());
        mdns_instance_name_set(Config::System::DEVICE_NAME.data());
        mdns_service_add(nullptr, "_http", "_tcp", 80, nullptr, 0);
        mdns_service_add(nullptr, "_ws", "_tcp", 80, nullptr, 0);

        std::printf("\033[1;32m[mDNS] Initialized: http://%.*s.local\033[0m\n",
                    static_cast<int>(Config::System::MDNS_HOSTNAME.length()),
                    Config::System::MDNS_HOSTNAME.data());
#else
        std::printf("\033[1;32m[mDNS] Local Hostname: http://%.*s.local\033[0m\n",
                    static_cast<int>(Config::System::MDNS_HOSTNAME.length()),
                    Config::System::MDNS_HOSTNAME.data());
#endif
        return Status::OK;
    }
};

// ============================================================================
// 2. SNTP NETWORK TIME SYNCHRONIZATION
// ============================================================================

template <typename Config>
class NTPEngine {
public:
    static Result<void> init() noexcept {
        if constexpr (!Config::Features::ENABLE_NTP_TIME_SYNC) return Status::OK;
#if defined(ESP_PLATFORM)
        esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, Config::Network::NTP_SERVER_1.data());
        esp_sntp_setservername(1, Config::Network::NTP_SERVER_2.data());
        esp_sntp_init();

        setenv("TZ", "WIB-7", 1);
        tzset();
        std::printf("\033[1;32m[NTP] Time Sync Initialized: Server %.*s (UTC+7)\033[0m\n",
                    static_cast<int>(Config::Network::NTP_SERVER_1.length()),
                    Config::Network::NTP_SERVER_1.data());
#endif
        return Status::OK;
    }

    [[nodiscard]] static bool is_time_synced() noexcept {
#if defined(ESP_PLATFORM)
        return esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED;
#else
        return true;
#endif
    }
};

// ============================================================================
// 3. ASYNC WEBSOCKET LIVE STREAMING ENGINE (WITH MULTI-SCHEME ENCRYPTION)
// ============================================================================

template <typename Config>
class AsyncWebSocketStreamer {
public:
    static void broadcast_telemetry(const UnifiedTelemetry& data) noexcept {
        if constexpr (!Config::Features::ENABLE_ASYNC_WEBSOCKET) return;

        FixedString<Config::Storage::JSON_STATIC_PAYLOAD_CAPACITY> raw_json;
        data.serialize_json(raw_json);

        // Apply Configured Encryption Scheme (AES-128 / ChaCha20 / HMAC / XOR / Plaintext)
        FixedString<Config::Storage::JSON_STATIC_PAYLOAD_CAPACITY> encrypted_payload;
        security::PayloadSecurityEngine::encrypt(raw_json.string_view(), encrypted_payload);

        // Broadcast over WebSocket (WS/WSS) clients
    }
};

// ============================================================================
// 4. LOCAL REST & WEBSOCKET SECURITY AUTHENTICATOR
// ============================================================================

class LocalSecurity {
public:
    static bool verify_token(std::string_view provided_token, std::string_view secret_key) noexcept {
        return crypto::constant_time_equals(provided_token, secret_key);
    }
};

} // namespace iot::protocols
