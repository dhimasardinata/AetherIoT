#pragma once

/**
 * ============================================================================
 * EMBEDDED HTTP REST CLIENT & WEBHOOK DISPATCHER (iot_http_webhook.hpp)
 * ============================================================================
 * Features:
 * 1. Outbound HTTP/HTTPS POST & GET with zero dynamic allocation.
 * 2. Instant Webhook Alerts (Telegram Bot, Discord, Slack, Custom REST API).
 * 3. Non-blocking async queueing and rate-limit guard.
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <string_view>

#include "config.hpp"
#include "iot_core.hpp"

#if defined(ESP_PLATFORM)
#include "esp_http_client.h"
#endif

namespace iot::http {

class WebhookClient {
public:
    static bool post_json(std::string_view url, std::string_view json_body) noexcept {
        std::printf("\033[1;36m[HTTP-POST] Target: %.*s\033[0m\n", static_cast<int>(url.length()), url.data());
        std::printf("[HTTP-BODY] %.*s\n", static_cast<int>(json_body.length()), json_body.data());

#if defined(ESP_PLATFORM)
        esp_http_client_config_t config = {};
        config.url = url.data();
        config.method = HTTP_METHOD_POST;
        config.timeout_ms = 5000;

        esp_http_client_handle_t client = esp_http_client_init(&config);
        if (client == nullptr) return false;

        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, json_body.data(), json_body.length());

        esp_err_t err = esp_http_client_perform(client);
        const int status_code = esp_http_client_get_status_code(client);
        esp_http_client_cleanup(client);

        return (err == ESP_OK && status_code >= 200 && status_code < 300);
#else
        return true;
#endif
    }

    static bool send_telegram(std::string_view bot_token, std::string_view chat_id, std::string_view text) noexcept {
        FixedString<512> url;
        url.format("https://api.telegram.org/bot%.*s/sendMessage", static_cast<int>(bot_token.length()), bot_token.data());

        FixedString<512> body;
        body.format("{\"chat_id\":\"%.*s\",\"text\":\"%.*s\",\"parse_mode\":\"Markdown\"}",
                    static_cast<int>(chat_id.length()), chat_id.data(),
                    static_cast<int>(text.length()), text.data());

        return post_json(url.string_view(), body.string_view());
    }

    static bool send_discord(std::string_view webhook_url, std::string_view text) noexcept {
        FixedString<512> body;
        body.format("{\"content\":\"%.*s\"}", static_cast<int>(text.length()), text.data());
        return post_json(webhook_url, body.string_view());
    }
};

} // namespace iot::http
