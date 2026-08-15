#pragma once

/**
 * ============================================================================
 * UNIFIED DUAL-ENGINE OTA FIRMWARE MANAGER (iot_ota.hpp)
 * ============================================================================
 * 1. Cloud HTTPS Auto-OTA: Checks cloud version, streams binary with SHA-256 check.
 * 2. In-Browser Local Web OTA: Drag-and-drop .bin upload directly via Web Dashboard.
 * 3. Self-Healing Rollback Protection: Reverts partition if new firmware panics.
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

#if defined(ESP_PLATFORM)
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#endif

namespace iot::ota {

template <typename Config>
class OTAManager {
public:
    enum class State : uint8_t {
        IDLE,
        CHECKING,
        DOWNLOADING,
        VERIFYING,
        APPLYING,
        FAILED,
        SUCCESS_REBOOTING
    };

    struct Progress {
        State    state{State::IDLE};
        uint32_t bytes_written{0};
        uint32_t total_bytes{0};
        uint8_t  percentage{0};
        char     status_msg[64]{"Idle"};
    };

    static Progress get_progress() noexcept {
        return progress_;
    }

    // ------------------------------------------------------------------------
    // 1. CLOUD HTTPS AUTO-OTA UPDATER
    // ------------------------------------------------------------------------
    static Result<void> check_and_update_from_cloud(std::string_view update_url) noexcept {
        if constexpr (!Config::Features::ENABLE_OTA) return Status::OK;

        std::printf("\033[1;36m[OTA-CLOUD] Checking Cloud Firmware at: %.*s\033[0m\n",
                    static_cast<int>(update_url.length()), update_url.data());

        progress_.state = State::CHECKING;
        std::snprintf(progress_.status_msg, sizeof(progress_.status_msg), "Connecting to Cloud OTA...");

#if defined(ESP_PLATFORM)
        esp_http_client_config_t http_config{};
        http_config.url = update_url.data();
        http_config.timeout_ms = 15000;
        http_config.keep_alive_enable = true;

        esp_https_ota_config_t ota_config{};
        ota_config.http_config = &http_config;

        esp_https_ota_handle_t https_ota_handle = nullptr;
        esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
        if (err != ESP_OK) {
            progress_.state = State::FAILED;
            std::snprintf(progress_.status_msg, sizeof(progress_.status_msg), "Failed to start HTTPS OTA");
            return Status::ERROR_HARDWARE_FAIL;
        }

        progress_.state = State::DOWNLOADING;
        progress_.total_bytes = esp_https_ota_get_image_size(https_ota_handle);

        while (true) {
            err = esp_https_ota_perform(https_ota_handle);
            if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
                break;
            }
            progress_.bytes_written = esp_https_ota_get_image_len_read(https_ota_handle);
            if (progress_.total_bytes > 0) {
                progress_.percentage = static_cast<uint8_t>((progress_.bytes_written * 100) / progress_.total_bytes);
            }
        }

        if (err == ESP_OK) {
            err = esp_https_ota_finish(https_ota_handle);
            if (err == ESP_OK) {
                progress_.state = State::SUCCESS_REBOOTING;
                progress_.percentage = 100;
                std::snprintf(progress_.status_msg, sizeof(progress_.status_msg), "OTA Success! Rebooting...");
                std::printf("\033[1;32m[OTA-CLOUD] Update Verified. Rebooting to new firmware partition...\033[0m\n");
                vTaskDelay(pdMS_TO_TICKS(1000));
                esp_restart();
            }
        }

        esp_https_ota_abort(https_ota_handle);
        progress_.state = State::FAILED;
        std::snprintf(progress_.status_msg, sizeof(progress_.status_msg), "OTA verification failed");
        return Status::ERROR_OTA_VERIFY;
#else
        progress_.percentage = 100;
        progress_.state = State::SUCCESS_REBOOTING;
        return Status::OK;
#endif
    }

    // ------------------------------------------------------------------------
    // 2. LOCAL IN-BROWSER WEB OTA ENGINE (ZERO CLOUD DEPENDENCY)
    // ------------------------------------------------------------------------
    static Result<void> begin_local_stream(size_t total_image_size) noexcept {
        progress_.state = State::DOWNLOADING;
        progress_.bytes_written = 0;
        progress_.total_bytes = static_cast<uint32_t>(total_image_size);
        progress_.percentage = 0;

#if defined(ESP_PLATFORM)
        const esp_partition_t* next_part = esp_ota_get_next_update_partition(nullptr);
        if (!next_part) return Status::ERROR_NOT_FOUND;

        esp_err_t err = esp_ota_begin(next_part, OTA_WITH_SEQUENTIAL_WRITES, &ota_handle_);
        if (err != ESP_OK) return Status::ERROR_HARDWARE_FAIL;
#endif
        return Status::OK;
    }

    static Result<void> write_local_chunk(std::span<const uint8_t> chunk) noexcept {
#if defined(ESP_PLATFORM)
        if (ota_handle_ == 0) return Status::ERROR_UNINITIALIZED;

        esp_err_t err = esp_ota_write(ota_handle_, chunk.data(), chunk.size());
        if (err != ESP_OK) return Status::ERROR_HARDWARE_FAIL;

        progress_.bytes_written += chunk.size();
        if (progress_.total_bytes > 0) {
            progress_.percentage = static_cast<uint8_t>((progress_.bytes_written * 100) / progress_.total_bytes);
        }
#else
        progress_.bytes_written += chunk.size();
#endif
        return Status::OK;
    }

    static Result<void> finalize_local_stream() noexcept {
#if defined(ESP_PLATFORM)
        if (ota_handle_ == 0) return Status::ERROR_UNINITIALIZED;

        esp_err_t err = esp_ota_end(ota_handle_);
        if (err != ESP_OK) return Status::ERROR_OTA_VERIFY;

        const esp_partition_t* next_part = esp_ota_get_next_update_partition(nullptr);
        err = esp_ota_set_boot_partition(next_part);
        if (err != ESP_OK) return Status::ERROR_HARDWARE_FAIL;

        progress_.state = State::SUCCESS_REBOOTING;
        std::printf("\033[1;32m[OTA-LOCAL] Web Firmware Flashed Successfully. Rebooting...\033[0m\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();
#endif
        return Status::OK;
    }

    // ------------------------------------------------------------------------
    // 3. POST-BOOT SELF-HEALING CONFIRMATION
    // ------------------------------------------------------------------------
    static void confirm_running_partition_valid() noexcept {
#if defined(ESP_PLATFORM)
        const esp_partition_t* running = esp_ota_get_running_partition();
        esp_ota_img_states_t ota_state;
        if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
            if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
                esp_ota_mark_app_valid_cancel_rollback();
                std::printf("\033[1;32m[OTA-GUARD] New Firmware Validated & Rollback Cancelled Successfully.\033[0m\n");
            }
        }
#endif
    }

private:
    static inline Progress progress_{};
#if defined(ESP_PLATFORM)
    static inline esp_ota_handle_t ota_handle_{0};
#endif
};

} // namespace iot::ota
