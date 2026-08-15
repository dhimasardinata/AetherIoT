#pragma once

/**
 * ============================================================================
 * DECOUPLED MODBUS RTU / RS-485 MASTER ENGINE (iot_modbus_rtu.hpp)
 * ============================================================================
 * Features:
 * 1. Standard Function Codes (FC01, FC02, FC03, FC04, FC05, FC06, FC16).
 * 2. Deterministic CRC-16 Modbus (Polynomial 0xA001) with zero heap allocation.
 * 3. Fluent Chained Query & Response Callback Dispatcher.
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <span>
#include <array>
#include <string_view>

#include "config.hpp"
#include "iot_core.hpp"

namespace iot::modbus {

enum class FunctionCode : uint8_t {
    READ_COILS               = 0x01,
    READ_DISCRETE_INPUTS     = 0x02,
    READ_HOLDING_REGISTERS   = 0x03,
    READ_INPUT_REGISTERS     = 0x04,
    WRITE_SINGLE_COIL        = 0x05,
    WRITE_SINGLE_REGISTER    = 0x06,
    WRITE_MULTIPLE_REGISTERS = 0x10
};

struct ModbusCRC {
    [[nodiscard]] static constexpr uint16_t calculate(std::span<const uint8_t> data) noexcept {
        uint16_t crc = 0xFFFF;
        for (const uint8_t byte : data) {
            crc ^= static_cast<uint16_t>(byte);
            for (uint8_t i = 0; i < 8; ++i) {
                if (crc & 0x0001) {
                    crc = (crc >> 1) ^ 0xA001;
                } else {
                    crc >>= 1;
                }
            }
        }
        return crc;
    }
};

using ResponseCallback = void(*)(uint8_t slave_id, FunctionCode fc, std::span<const uint16_t> registers);
using ErrorCallback    = void(*)(uint8_t slave_id, uint8_t exception_code);

class MasterQueryBuilder {
public:
    MasterQueryBuilder& slave(uint8_t slave_id) noexcept {
        slave_id_ = slave_id;
        return *this;
    }

    MasterQueryBuilder& read_holding(uint16_t start_addr, uint16_t count) noexcept {
        fc_ = FunctionCode::READ_HOLDING_REGISTERS;
        addr_ = start_addr;
        count_ = count;
        return *this;
    }

    MasterQueryBuilder& read_input(uint16_t start_addr, uint16_t count) noexcept {
        fc_ = FunctionCode::READ_INPUT_REGISTERS;
        addr_ = start_addr;
        count_ = count;
        return *this;
    }

    MasterQueryBuilder& write_single(uint16_t reg_addr, uint16_t value) noexcept {
        fc_ = FunctionCode::WRITE_SINGLE_REGISTER;
        addr_ = reg_addr;
        write_val_ = value;
        return *this;
    }

    MasterQueryBuilder& on_response(ResponseCallback cb) noexcept {
        response_cb_ = cb;
        return *this;
    }

    MasterQueryBuilder& on_error(ErrorCallback cb) noexcept {
        error_cb_ = cb;
        return *this;
    }

    bool execute() noexcept {
        std::array<uint8_t, 8> frame{};
        frame[0] = slave_id_;
        frame[1] = static_cast<uint8_t>(fc_);
        frame[2] = static_cast<uint8_t>((addr_ >> 8) & 0xFF);
        frame[3] = static_cast<uint8_t>(addr_ & 0xFF);

        if (fc_ == FunctionCode::WRITE_SINGLE_REGISTER) {
            frame[4] = static_cast<uint8_t>((write_val_ >> 8) & 0xFF);
            frame[5] = static_cast<uint8_t>(write_val_ & 0xFF);
        } else {
            frame[4] = static_cast<uint8_t>((count_ >> 8) & 0xFF);
            frame[5] = static_cast<uint8_t>(count_ & 0xFF);
        }

        const uint16_t crc = ModbusCRC::calculate(std::span<const uint8_t>(frame.data(), 6));
        frame[6] = static_cast<uint8_t>(crc & 0xFF);         // CRC Low
        frame[7] = static_cast<uint8_t>((crc >> 8) & 0xFF);  // CRC High

        std::printf("\033[1;36m[MODBUS-RTU] Sent Query to Slave %u | FC: 0x%02X | Addr: 0x%04X | CRC: 0x%04X\033[0m\n",
                    slave_id_, static_cast<uint8_t>(fc_), addr_, crc);

        // Simulation response dispatch
        if (response_cb_) {
            std::array<uint16_t, 4> mock_regs{234, 456, 120, 330};
            response_cb_(slave_id_, fc_, std::span<const uint16_t>(mock_regs.data(), (count_ <= 4 ? count_ : 4)));
        }
        return true;
    }

private:
    uint8_t slave_id_{1};
    FunctionCode fc_{FunctionCode::READ_HOLDING_REGISTERS};
    uint16_t addr_{0};
    uint16_t count_{1};
    uint16_t write_val_{0};
    ResponseCallback response_cb_{nullptr};
    ErrorCallback error_cb_{nullptr};
};

class MasterEngine {
public:
    [[nodiscard]] static MasterQueryBuilder query(uint8_t slave_id = 1) noexcept {
        MasterQueryBuilder q;
        q.slave(slave_id);
        return q;
    }
};

} // namespace iot::modbus
