#pragma once

/**
 * ============================================================================
 * FLASH PARTITION TABLE & MEMORY GEOMETRY INSPECTOR (iot_partitions.hpp)
 * ============================================================================
 * Features:
 * 1. Runtime Flash Partition Query (Running app, OTA next, NVS, Storage).
 * 2. Flash Chip Geometry (Size in MB, Speed in MHz, Mode).
 * 3. Memory Map Diagnostics Printer.
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <string_view>
#include <span>
#include <array>

#include "config.hpp"
#include "iot_core.hpp"

#if defined(ESP_PLATFORM)
#include "esp_partition.h"
#include "esp_flash.h"
#include "esp_ota_ops.h"
#endif

namespace iot::partitions {

struct PartitionInfo {
    FixedString<16> label{};
    uint8_t  type{0};
    uint8_t  subtype{0};
    uint32_t address{0};
    uint32_t size_bytes{0};
    bool     is_encrypted{false};
};

class PartitionManager {
public:
    static void print_memory_map() noexcept {
        std::printf("\n\033[1;36m===============================================================\033[0m\n");
        std::printf("\033[1;32m[FLASH-MAP] Flash Partition Table & Memory Allocation\033[0m\n");
        std::printf("\033[1;36m===============================================================\033[0m\n");
        std::printf("  Name       | Type | SubType | Address    | Size (KB)  | Size (MB) \n");
        std::printf("-------------+------+---------+------------+------------+-----------\n");

#if defined(ESP_PLATFORM)
        esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, nullptr);
        while (it != nullptr) {
            const esp_partition_t* p = esp_partition_get(it);
            if (p != nullptr) {
                std::printf("  %-10s | 0x%02X | 0x%02X    | 0x%08lX | %7lu KB | %4.1f MB\n",
                            p->label, p->type, p->subtype,
                            static_cast<unsigned long>(p->address),
                            static_cast<unsigned long>(p->size / 1024),
                            static_cast<float>(p->size) / (1024.0f * 1024.0f));
            }
            it = esp_partition_next(it);
        }
        esp_partition_iterator_release(it);
#else
        std::printf("  nvs        | 0x01 | 0x02    | 0x00009000 |      20 KB |  0.0 MB\n");
        std::printf("  otadata    | 0x01 | 0x00    | 0x0000E000 |       8 KB |  0.0 MB\n");
        std::printf("  app0 (OTA) | 0x00 | 0x10    | 0x00010000 |    1536 KB |  1.5 MB\n");
        std::printf("  app1 (OTA) | 0x00 | 0x11    | 0x00190000 |    1536 KB |  1.5 MB\n");
        std::printf("  storage    | 0x01 | 0x82    | 0x00310000 |     896 KB |  0.9 MB\n");
#endif
        std::printf("\033[1;36m===============================================================\033[0m\n\n");
    }

    [[nodiscard]] static uint32_t flash_chip_size_bytes() noexcept {
#if defined(ESP_PLATFORM)
        uint32_t size = 0;
        if (esp_flash_get_size(nullptr, &size) == ESP_OK) {
            return size;
        }
        return 4 * 1024 * 1024;
#else
        return 4 * 1024 * 1024; // 4MB Default in Simulation
#endif
    }

    [[nodiscard]] static uint32_t flash_chip_size_mb() noexcept {
        return flash_chip_size_bytes() / (1024 * 1024);
    }

    [[nodiscard]] static std::string_view running_app_label() noexcept {
#if defined(ESP_PLATFORM)
        const esp_partition_t* running = esp_ota_get_running_partition();
        if (running != nullptr) {
            return running->label;
        }
#endif
        return "app0";
    }

    [[nodiscard]] static std::string_view next_ota_update_label() noexcept {
#if defined(ESP_PLATFORM)
        const esp_partition_t* next = esp_ota_get_next_update_partition(nullptr);
        if (next != nullptr) {
            return next->label;
        }
#endif
        return "app1";
    }
};

} // namespace iot::partitions
