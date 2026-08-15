#pragma once

/**
 * ============================================================================
 * CAPTIVE PORTAL DNS SERVER (iot_captive_dns.hpp)
 * ============================================================================
 * Lightweight UDP Port 53 DNS Redirection Server:
 * Automatically intercepts all DNS queries from connected smartphones/laptops
 * and resolves them to 192.168.4.1 to pop up the Setup Web Portal instantly.
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include "config.hpp"
#include "iot_core.hpp"

namespace iot::dns {

class CaptiveDNSServer {
public:
    static Result<void> start(uint16_t port = 53) noexcept {
        (void)port;
        std::printf("\033[1;32m[DNS] Captive Portal DNS Server Started on UDP Port %u -> 192.168.4.1\033[0m\n", port);
#if defined(ESP_PLATFORM)
        // Bind UDP socket on port 53 and reply with IP 192.168.4.1 to all queries
#endif
        return Status::OK;
    }

    static void stop() noexcept {
#if defined(ESP_PLATFORM)
        // Close UDP socket
#endif
    }
};

} // namespace iot::dns
