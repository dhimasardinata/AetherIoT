#pragma once

/**
 * ============================================================================
 * GATEWAY CONTROL STATE & MANUAL OVERRIDE (iot_control_mode.hpp)
 * ============================================================================
 * Prevents automatic automation rules from conflicting with manual technician
 * overrides or maintenance safety lockouts:
 * - AUTO: Normal rule-based automated control
 * - MANUAL_OVERRIDE: Automated rules suspended for technician manual testing
 * - SAFETY_LOCKOUT: Complete hardware interlock (all relays forced OFF)
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <string_view>
#include "config.hpp"
#include "iot_core.hpp"

namespace iot::control {

enum class Mode : uint8_t {
    AUTO = 0,
    MANUAL_OVERRIDE = 1,
    SAFETY_LOCKOUT = 2
};

[[nodiscard]] constexpr std::string_view mode_to_string(Mode m) noexcept {
    switch (m) {
        case Mode::AUTO:            return "AUTO (Automated Rules Active)";
        case Mode::MANUAL_OVERRIDE: return "MANUAL_OVERRIDE (Rules Suspended)";
        case Mode::SAFETY_LOCKOUT:  return "SAFETY_LOCKOUT (Interlock Active)";
        default:                    return "UNKNOWN";
    }
}

class ControlStateManager {
public:
    static void set_mode(Mode m, uint32_t manual_timeout_ms = 300000) noexcept {
        current_mode_ = m;
        manual_timeout_ms_ = manual_timeout_ms;
        std::printf("\033[1;35m[CONTROL-MODE] Gateway Switched to: %.*s\033[0m\n",
                    static_cast<int>(mode_to_string(m).length()),
                    mode_to_string(m).data());
    }

    [[nodiscard]] static Mode get_mode() noexcept { return current_mode_; }
    [[nodiscard]] static bool is_auto() noexcept { return current_mode_ == Mode::AUTO; }
    [[nodiscard]] static bool is_manual() noexcept { return current_mode_ == Mode::MANUAL_OVERRIDE; }
    [[nodiscard]] static bool is_lockout() noexcept { return current_mode_ == Mode::SAFETY_LOCKOUT; }

private:
    static inline Mode current_mode_{Mode::AUTO};
    static inline uint32_t manual_timeout_ms_{300000}; // 5 min default manual override
};

} // namespace iot::control
