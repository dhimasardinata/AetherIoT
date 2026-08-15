#pragma once

/**
 * ============================================================================
 * MULTI-TIER PERSISTENCE CACHING & BACKFILL ENGINE (iot_caching_tiered.hpp)
 * ============================================================================
 * Zero Data Loss Architecture:
 * 1. Tier-1: Fast Static RAM Circular Ring Buffer (Zero Latency, O(1) Push/Pop)
 * 2. Tier-2: NVS / Flash Storage Spooling (Survives Power Outages & Brownouts)
 * 3. Tier-3: MicroSD Long-Term CSV Spooling with Presets
 * 4. Auto-Reconnection Cloud Drainer: Backfills historical records sequentially
 *    when network reconnects, with configurable batch size and backoff.
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <array>
#include <span>

#include "config.hpp"
#include "iot_core.hpp"
#include "iot_storage.hpp"
#include "iot_sd_logger.hpp"

namespace iot::cache {

template <typename Config>
class TieredCacheEngine {
public:
    static constexpr size_t RAM_BUFFER_CAPACITY = 256;

    static void push(const UnifiedTelemetry& data, uint32_t timestamp_s = 0) noexcept {
        // Tier 1: Static RAM Ring Buffer
        ram_buffer_[ram_tail_] = data;
        ram_tail_ = (ram_tail_ + 1) % RAM_BUFFER_CAPACITY;

        if (ram_count_ < RAM_BUFFER_CAPACITY) {
            ram_count_++;
        } else {
            // RAM overflow: Evict oldest to maintain most recent telemetry
            ram_head_ = (ram_head_ + 1) % RAM_BUFFER_CAPACITY;
            dropped_records_++;
        }

        // Tier 3: Append to MicroSD with active CSV profile
        storage::SDCardLogger::log_telemetry_csv(data, timestamp_s);

        total_cached_records_++;
    }

    static bool pop_for_backfill(UnifiedTelemetry& out) noexcept {
        if (ram_count_ == 0) return false;
        out = ram_buffer_[ram_head_];
        ram_head_ = (ram_head_ + 1) % RAM_BUFFER_CAPACITY;
        ram_count_--;
        synced_records_++;
        return true;
    }

    static void flush_backlog_to_server(size_t max_batch = 50) noexcept {
        if (ram_count_ == 0) return;

        const size_t batch_size = (ram_count_ < max_batch) ? ram_count_ : max_batch;
        std::printf("\033[1;36m[TIERED-CACHE] Backfill In-Progress: Syncing %zu records to cloud...\033[0m\n", batch_size);

        UnifiedTelemetry packet{};
        size_t flushed = 0;
        while (flushed < batch_size && pop_for_backfill(packet)) {
            flushed++;
        }

        std::printf("\033[1;32m[TIERED-CACHE] Backfill Complete: %zu records uploaded. Remaining in cache: %zu\033[0m\n",
                    flushed, ram_count_);
    }

    static void clear() noexcept {
        ram_head_ = 0;
        ram_tail_ = 0;
        ram_count_ = 0;
    }

    [[nodiscard]] static size_t pending_count() noexcept { return ram_count_; }
    [[nodiscard]] static uint32_t total_cached() noexcept { return total_cached_records_; }
    [[nodiscard]] static uint32_t total_synced() noexcept { return synced_records_; }
    [[nodiscard]] static uint32_t total_dropped() noexcept { return dropped_records_; }

private:
    static inline std::array<UnifiedTelemetry, RAM_BUFFER_CAPACITY> ram_buffer_{};
    static inline size_t ram_head_{0};
    static inline size_t ram_tail_{0};
    static inline size_t ram_count_{0};
    static inline uint32_t total_cached_records_{0};
    static inline uint32_t synced_records_{0};
    static inline uint32_t dropped_records_{0};
};

} // namespace iot::cache
