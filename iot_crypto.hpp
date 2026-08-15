#pragma once

/**
 * ============================================================================
 * ZERO-HEAP CRYPTO & SECURITY ENGINE (iot_crypto.hpp)
 * ============================================================================
 * High-Performance Bare-Metal Cryptographic Suite:
 * - SHA-256 Hash Engine (Zero Dynamic Allocation)
 * - HMAC-SHA256 Signature Generator & Token Verifier
 * - Base64 Zero-Allocation Encoder & Decoder
 * - Constant-Time Memory Comparison (Timing-Attack Resistant)
 * ============================================================================
 */

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>
#include <array>
#include <string_view>

#include "iot_core.hpp"

namespace iot::crypto {

// ============================================================================
// 1. CONSTANT-TIME MEMORY COMPARATOR (TIMING-ATTACK RESISTANT)
// ============================================================================

[[nodiscard]] inline bool constant_time_equals(std::span<const uint8_t> a, std::span<const uint8_t> b) noexcept {
    if (a.size() != b.size()) return false;
    uint8_t result = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        result |= (a[i] ^ b[i]);
    }
    return result == 0;
}

[[nodiscard]] inline bool constant_time_equals(std::string_view a, std::string_view b) noexcept {
    if (a.size() != b.size()) return false;
    uint8_t result = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        result |= (static_cast<uint8_t>(a[i]) ^ static_cast<uint8_t>(b[i]));
    }
    return result == 0;
}

// ============================================================================
// 2. SHA-256 SECURE HASH ENGINE (ZERO HEAP)
// ============================================================================

class SHA256 {
public:
    static constexpr size_t DIGEST_SIZE = 32;
    static constexpr size_t BLOCK_SIZE  = 64;

    constexpr SHA256() noexcept { reset(); }

    constexpr void reset() noexcept {
        state_[0] = 0x6a09e667;
        state_[1] = 0xbb67ae85;
        state_[2] = 0x3c6ef372;
        state_[3] = 0xa54ff53a;
        state_[4] = 0x510e527f;
        state_[5] = 0x9b05688c;
        state_[6] = 0x1f83d9ab;
        state_[7] = 0x5be0cd19;
        count_ = 0;
        buffer_len_ = 0;
    }

    void update(std::span<const uint8_t> data) noexcept {
        for (const uint8_t byte : data) {
            buffer_[buffer_len_++] = byte;
            if (buffer_len_ == BLOCK_SIZE) {
                transform(buffer_.data());
                count_ += 512;
                buffer_len_ = 0;
            }
        }
    }

    void update(std::string_view str) noexcept {
        update(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(str.data()), str.size()));
    }

    [[nodiscard]] std::array<uint8_t, DIGEST_SIZE> finalize() noexcept {
        count_ += buffer_len_ * 8;
        buffer_[buffer_len_++] = 0x80;

        if (buffer_len_ > 56) {
            while (buffer_len_ < 64) buffer_[buffer_len_++] = 0x00;
            transform(buffer_.data());
            buffer_len_ = 0;
        }

        while (buffer_len_ < 56) buffer_[buffer_len_++] = 0x00;

        for (int i = 7; i >= 0; --i) {
            buffer_[56 + (7 - i)] = static_cast<uint8_t>((count_ >> (i * 8)) & 0xFF);
        }
        transform(buffer_.data());

        std::array<uint8_t, DIGEST_SIZE> digest{};
        for (size_t i = 0; i < 8; ++i) {
            digest[(i * 4) + 0] = static_cast<uint8_t>((state_[i] >> 24) & 0xFF);
            digest[(i * 4) + 1] = static_cast<uint8_t>((state_[i] >> 16) & 0xFF);
            digest[(i * 4) + 2] = static_cast<uint8_t>((state_[i] >> 8) & 0xFF);
            digest[(i * 4) + 3] = static_cast<uint8_t>(state_[i] & 0xFF);
        }
        reset();
        return digest;
    }

    static std::array<uint8_t, DIGEST_SIZE> hash(std::span<const uint8_t> data) noexcept {
        SHA256 ctx;
        ctx.update(data);
        return ctx.finalize();
    }

private:
    static constexpr uint32_t rotr(uint32_t x, uint32_t n) noexcept {
        return (x >> n) | (x << (32 - n));
    }

    static constexpr uint32_t ch(uint32_t x, uint32_t y, uint32_t z) noexcept {
        return (x & y) ^ (~x & z);
    }

    static constexpr uint32_t maj(uint32_t x, uint32_t y, uint32_t z) noexcept {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    static constexpr uint32_t sigma0(uint32_t x) noexcept {
        return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
    }

    static constexpr uint32_t sigma1(uint32_t x) noexcept {
        return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
    }

    static constexpr uint32_t gamma0(uint32_t x) noexcept {
        return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
    }

    static constexpr uint32_t gamma1(uint32_t x) noexcept {
        return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
    }

    void transform(const uint8_t* chunk) noexcept {
        static constexpr std::array<uint32_t, 64> K = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };

        std::array<uint32_t, 64> w{};
        for (size_t i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(chunk[i * 4]) << 24) |
                   (static_cast<uint32_t>(chunk[(i * 4) + 1]) << 16) |
                   (static_cast<uint32_t>(chunk[(i * 4) + 2]) << 8) |
                   static_cast<uint32_t>(chunk[(i * 4) + 3]);
        }
        for (size_t i = 16; i < 64; ++i) {
            w[i] = gamma1(w[i - 2]) + w[i - 7] + gamma0(w[i - 15]) + w[i - 16];
        }

        uint32_t a = state_[0];
        uint32_t b = state_[1];
        uint32_t c = state_[2];
        uint32_t d = state_[3];
        uint32_t e = state_[4];
        uint32_t f = state_[5];
        uint32_t g = state_[6];
        uint32_t h = state_[7];

        for (size_t i = 0; i < 64; ++i) {
            const uint32_t t1 = h + sigma1(e) + ch(e, f, g) + K[i] + w[i];
            const uint32_t t2 = sigma0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        state_[0] += a;
        state_[1] += b;
        state_[2] += c;
        state_[3] += d;
        state_[4] += e;
        state_[5] += f;
        state_[6] += g;
        state_[7] += h;
    }

    std::array<uint32_t, 8> state_{};
    std::array<uint8_t, 64> buffer_{};
    uint64_t count_{0};
    size_t   buffer_len_{0};
};

// ============================================================================
// 3. HMAC-SHA256 SIGNATURE ENGINE
// ============================================================================

class HMAC_SHA256 {
public:
    static std::array<uint8_t, SHA256::DIGEST_SIZE> generate(std::span<const uint8_t> key, std::span<const uint8_t> data) noexcept {
        std::array<uint8_t, SHA256::BLOCK_SIZE> k_pad{};

        if (key.size() > SHA256::BLOCK_SIZE) {
            const auto key_hash = SHA256::hash(key);
            std::memcpy(k_pad.data(), key_hash.data(), SHA256::DIGEST_SIZE);
        } else {
            std::memcpy(k_pad.data(), key.data(), key.size());
        }

        std::array<uint8_t, SHA256::BLOCK_SIZE> i_pad{};
        for (size_t i = 0; i < SHA256::BLOCK_SIZE; ++i) i_pad[i] = k_pad[i] ^ 0x36;

        SHA256 inner_ctx;
        inner_ctx.update(i_pad);
        inner_ctx.update(data);
        const auto inner_hash = inner_ctx.finalize();

        std::array<uint8_t, SHA256::BLOCK_SIZE> o_pad{};
        for (size_t i = 0; i < SHA256::BLOCK_SIZE; ++i) o_pad[i] = k_pad[i] ^ 0x5C;

        SHA256 outer_ctx;
        outer_ctx.update(o_pad);
        outer_ctx.update(inner_hash);
        return outer_ctx.finalize();
    }
};

// ============================================================================
// 4. BASE-64 ZERO-ALLOCATION ENCODER & DECODER
// ============================================================================

class Base64 {
public:
    static constexpr std::string_view TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    template <size_t OutCapacity>
    static bool encode(std::span<const uint8_t> input, FixedString<OutCapacity>& output) noexcept {
        output.clear();
        const size_t len = input.size();
        for (size_t i = 0; i < len; i += 3) {
            const uint32_t b0 = input[i];
            const uint32_t b1 = (i + 1 < len) ? input[i + 1] : 0;
            const uint32_t b2 = (i + 2 < len) ? input[i + 2] : 0;

            const uint32_t triple = (b0 << 16) | (b1 << 8) | b2;

            if (!output.append_char(TABLE[(triple >> 18) & 0x3F])) return false;
            if (!output.append_char(TABLE[(triple >> 12) & 0x3F])) return false;
            if (!output.append_char((i + 1 < len) ? TABLE[(triple >> 6) & 0x3F] : '=')) return false;
            if (!output.append_char((i + 2 < len) ? TABLE[triple & 0x3F] : '=')) return false;
        }
        return true;
    }
};

} // namespace iot::crypto
