#pragma once

/**
 * ============================================================================
 * NON-VOLATILE FLASH PERSISTENCE ENGINE (iot_storage.hpp)
 * ============================================================================
 * Manages persistent storage of runtime calibration, dosing history, and
 * security credentials with CRC32 integrity verification and brownout protection.
 * ============================================================================
 */

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>
#include <array>

#include "config.hpp"
#include "iot_core.hpp"

#if defined(ESP_PLATFORM)
#include "nvs_flash.h"
#include "nvs.h"
#endif

namespace iot::storage {

struct PersistentRecord {
    uint32_t magic{0x554E4946}; // "UNIF"
    uint32_t schema_version{1};
    uint32_t revision{0};

    // Calibration Offsets
    float    ph_slope{1.0f};
    float    ph_offset{0.0f};
    float    ec_slope{1.0f};
    float    ec_offset{0.0f};
    int32_t  water_level_offset_mm{0};

    // Dosing History
    float    accumulated_dose_today_ml{0.0f};
    uint32_t last_day_of_year{0};

    // CRC32 Checksum of preceding fields
    uint32_t crc32_checksum{0};
};

template <typename Config>
class NVSStorageManager {
public:
    static Result<void> init() noexcept {
#if defined(ESP_PLATFORM)
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            nvs_flash_erase();
            err = nvs_flash_init();
        }
        return (err == ESP_OK) ? Status::OK : Status::ERROR_HARDWARE_FAIL;
#else
        return Status::OK;
#endif
    }

    static Result<PersistentRecord> load() noexcept {
        PersistentRecord record{};
#if defined(ESP_PLATFORM)
        nvs_handle_t handle;
        if (nvs_open("sys_cfg", NVS_READONLY, &handle) != ESP_OK) {
            return Status::ERROR_NOT_FOUND;
        }

        size_t size = sizeof(PersistentRecord);
        esp_err_t err = nvs_get_blob(handle, "record", &record, &size);
        nvs_close(handle);

        if (err != ESP_OK || size != sizeof(PersistentRecord)) {
            return Status::ERROR_NOT_FOUND;
        }

        // Validate CRC32
        const uint32_t calc_crc = crc::calculate_crc32(
            std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&record), sizeof(PersistentRecord) - 4)
        );
        if (calc_crc != record.crc32_checksum || record.magic != 0x554E4946) {
            return Status::ERROR_CRC_MISMATCH;
        }
#else
        record.ph_slope = Config::Calibration::PH_SLOPE;
        record.ph_offset = Config::Calibration::PH_OFFSET;
        record.ec_slope = Config::Calibration::EC_SLOPE;
        record.ec_offset = Config::Calibration::EC_OFFSET;
#endif
        return record;
    }

    static Result<void> save(PersistentRecord& record) noexcept {
        record.magic = 0x554E4946;
        record.schema_version = 1;
        record.revision++;

        // Compute CRC32
        record.crc32_checksum = crc::calculate_crc32(
            std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&record), sizeof(PersistentRecord) - 4)
        );

#if defined(ESP_PLATFORM)
        nvs_handle_t handle;
        if (nvs_open("sys_cfg", NVS_READWRITE, &handle) != ESP_OK) {
            return Status::ERROR_HARDWARE_FAIL;
        }

        esp_err_t err = nvs_set_blob(handle, "record", &record, sizeof(PersistentRecord));
        if (err == ESP_OK) {
            err = nvs_commit(handle);
        }
        nvs_close(handle);
        return (err == ESP_OK) ? Status::OK : Status::ERROR_HARDWARE_FAIL;
#else
        return Status::OK;
#endif
    }
};

} // namespace iot::storage
