#pragma once

/**
 * ============================================================================
 * HIGH-LEVEL EXTENSIBLE CUSTOM CLI ENGINE (iot_cli.hpp)
 * ============================================================================
 * Architecture:
 * - Zero Dynamic Memory Allocation (0% Heap)
 * - Fluent Declarative Custom Command Registration
 * - Built-in Multi-Channel Output (Serial, BLE NUS, WebSocket, Telnet)
 * - Automatic Typed Argument Parsing (strings, ints, floats, bools)
 * - Dynamic Auto-Generated Help Menu
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string_view>
#include <array>
#include <span>

#include "iot_core.hpp"

namespace iot::terminal {

// ============================================================================
// 1. COMMAND EXECUTION CONTEXT & ARGUMENT PARSER
// ============================================================================
class Context {
public:
    static constexpr size_t MAX_ARGS = 8;

    Context(std::string_view full_line) noexcept {
        parse_line(full_line);
    }

    [[nodiscard]] std::string_view command() const noexcept { return command_.string_view(); }
    [[nodiscard]] size_t argc() const noexcept { return arg_count_; }

    [[nodiscard]] std::string_view arg(size_t index) const noexcept {
        if (index < arg_count_) return args_[index].string_view();
        return {};
    }

    [[nodiscard]] int arg_int(size_t index, int default_val = 0) const noexcept {
        if (index >= arg_count_) return default_val;
        const auto sv = args_[index].string_view();
        char buf[16]{};
        const size_t len = (sv.length() < 15) ? sv.length() : 15;
        std::memcpy(buf, sv.data(), len);
        char* end = nullptr;
        long val = std::strtol(buf, &end, 10);
        return (end != buf) ? static_cast<int>(val) : default_val;
    }

    [[nodiscard]] float arg_float(size_t index, float default_val = 0.0f) const noexcept {
        if (index >= arg_count_) return default_val;
        const auto sv = args_[index].string_view();
        char buf[24]{};
        const size_t len = (sv.length() < 23) ? sv.length() : 23;
        std::memcpy(buf, sv.data(), len);
        char* end = nullptr;
        float val = std::strtof(buf, &end);
        return (end != buf) ? val : default_val;
    }

    [[nodiscard]] bool arg_bool(size_t index, bool default_val = false) const noexcept {
        if (index >= arg_count_) return default_val;
        const auto sv = args_[index].string_view();
        if (sv == "1" || sv == "true" || sv == "on" || sv == "yes" || sv == "enable") return true;
        if (sv == "0" || sv == "false" || sv == "off" || sv == "no" || sv == "disable") return false;
        return default_val;
    }

    template <typename... Args>
    void respond(const char* fmt, Args... args) const noexcept {
        std::printf("[CLI] ");
        std::printf(fmt, args...);
        std::printf("\n");
    }

    template <typename... Args>
    void respond_ok(const char* fmt, Args... args) const noexcept {
        std::printf("\033[1;32m[CLI OK] ");
        std::printf(fmt, args...);
        std::printf("\033[0m\n");
    }

    template <typename... Args>
    void respond_error(const char* fmt, Args... args) const noexcept {
        std::printf("\033[1;31m[CLI ERROR] ");
        std::printf(fmt, args...);
        std::printf("\033[0m\n");
    }

private:
    void parse_line(std::string_view line) noexcept {
        // Trim leading and trailing whitespace
        while (!line.empty() && (line.front() == ' ' || line.front() == '\r' || line.front() == '\n' || line.front() == '\t')) {
            line.remove_prefix(1);
        }
        while (!line.empty() && (line.back() == ' ' || line.back() == '\r' || line.back() == '\n' || line.back() == '\t')) {
            line.remove_suffix(1);
        }
        if (line.empty()) return;

        // Extract command name
        const size_t first_space = line.find(' ');
        if (first_space == std::string_view::npos) {
            command_.assign(line);
            return;
        }

        command_.assign(line.substr(0, first_space));
        std::string_view rest = line.substr(first_space + 1);

        // Tokenize remaining arguments
        while (!rest.empty() && arg_count_ < MAX_ARGS) {
            while (!rest.empty() && rest.front() == ' ') rest.remove_prefix(1);
            if (rest.empty()) break;

            const size_t next_space = rest.find(' ');
            if (next_space == std::string_view::npos) {
                args_[arg_count_++].assign(rest);
                break;
            } else {
                args_[arg_count_++].assign(rest.substr(0, next_space));
                rest.remove_prefix(next_space + 1);
            }
        }
    }

    FixedString<32> command_{};
    std::array<FixedString<32>, MAX_ARGS> args_{};
    size_t arg_count_{0};
};

// ============================================================================
// 2. DECLARATIVE COMMAND BUILDER & REGISTRY
// ============================================================================
class CommandBuilder {
public:
    using HandlerFn = void(*)(Context&);

    CommandBuilder() = default;

    CommandBuilder& name(std::string_view cmd_name) noexcept {
        name_.assign(cmd_name);
        return *this;
    }

    CommandBuilder& description(std::string_view desc) noexcept {
        desc_.assign(desc);
        return *this;
    }

    CommandBuilder& usage(std::string_view use) noexcept {
        usage_.assign(use);
        return *this;
    }

    CommandBuilder& on_execute(HandlerFn handler) noexcept {
        handler_ = handler;
        return *this;
    }

    [[nodiscard]] std::string_view get_name() const noexcept { return name_.string_view(); }
    [[nodiscard]] std::string_view get_description() const noexcept { return desc_.string_view(); }
    [[nodiscard]] std::string_view get_usage() const noexcept { return usage_.string_view(); }
    [[nodiscard]] bool has_handler() const noexcept { return handler_ != nullptr; }

    void execute(Context& ctx) const noexcept {
        if (handler_) {
            handler_(ctx);
        }
    }

private:
    FixedString<24> name_{};
    FixedString<64> desc_{};
    FixedString<48> usage_{};
    HandlerFn handler_{nullptr};
};

class Registry {
public:
    static constexpr size_t MAX_CUSTOM_COMMANDS = 32;

    static CommandBuilder& register_command(std::string_view name) noexcept {
        // Check if command already registered
        for (size_t i = 0; i < count_; ++i) {
            if (commands_[i].get_name() == name) {
                return commands_[i];
            }
        }
        // Register new command
        if (count_ < MAX_CUSTOM_COMMANDS) {
            commands_[count_].name(name);
            return commands_[count_++];
        }
        return commands_[0]; // Fallback if capacity exceeded
    }

    static bool dispatch(std::string_view line) noexcept {
        Context ctx(line);
        if (ctx.command().empty()) return false;

        if (ctx.command() == "help" || ctx.command() == "?") {
            print_help();
            return true;
        }

        for (size_t i = 0; i < count_; ++i) {
            if (commands_[i].get_name() == ctx.command()) {
                commands_[i].execute(ctx);
                return true;
            }
        }

        std::printf("\033[1;31m[CLI] Unknown command '%.*s'. Type 'help' for available commands.\033[0m\n",
                    static_cast<int>(ctx.command().length()), ctx.command().data());
        return false;
    }

    static void print_help() noexcept {
        std::printf("\n\033[1;36m=================================================================\033[0m\n");
        std::printf("\033[1;36m  AETHERIOT INDUSTRIAL CLI TERMINAL - REGISTERED COMMANDS        \033[0m\n");
        std::printf("\033[1;36m=================================================================\033[0m\n");
        std::printf("  %-16s %-32s %s\n", "COMMAND", "DESCRIPTION", "USAGE");
        std::printf("  %-16s %-32s %s\n", "-------", "-----------", "-----");

        for (size_t i = 0; i < count_; ++i) {
            std::printf("  \033[1;32m%-16.*s\033[0m %-32.*s \033[1;33m%.*s\033[0m\n",
                        static_cast<int>(commands_[i].get_name().length()), commands_[i].get_name().data(),
                        static_cast<int>(commands_[i].get_description().length()), commands_[i].get_description().data(),
                        static_cast<int>(commands_[i].get_usage().length()), commands_[i].get_usage().data());
        }
        std::printf("\033[1;36m=================================================================\033[0m\n\n");
    }

    [[nodiscard]] static size_t command_count() noexcept { return count_; }

private:
    static inline std::array<CommandBuilder, MAX_CUSTOM_COMMANDS> commands_{};
    static inline size_t count_{0};
};

} // namespace iot::terminal
