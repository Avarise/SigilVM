#pragma once
#include <sigil/common.h>
#include <filesystem>
#include <cstdint>
#include <string>
#include <array>

namespace sigil::math {

// Generic Hash template
template <std::size_t Bits>
struct hash_t {
    static_assert(Bits > 0, "Hash size must be non-zero.");
    static_assert(Bits % 32 == 0, "Hash size must be multiple of 32 bits.");

    static constexpr std::size_t bit_count  = Bits;
    static constexpr std::size_t byte_count = Bits / 8;
    static constexpr std::size_t word_count = Bits / 32;

    using storage_type = std::array<std::byte, byte_count>;

    storage_type bytes{};

    // ---- Construction ----

    constexpr hash_t() noexcept = default;

    constexpr explicit hash_t(const storage_type& data) noexcept
        : bytes(data)
    {}

    // ---- Access ----

    constexpr const std::byte* data() const noexcept
    {
        return bytes.data();
    }

    constexpr std::byte* data() noexcept
    {
        return bytes.data();
    }

    constexpr std::size_t size() const noexcept
    {
        return byte_count;
    }

    // ---- Comparison ----

    constexpr bool operator==(const hash_t&) const noexcept = default;
    constexpr auto operator<=>(const hash_t&) const noexcept = default;

    // ---- Utility ----

    constexpr bool is_zero() const noexcept
    {
        for (auto b : bytes)
            if (b != std::byte{0})
                return false;
        return true;
    }
};

struct sha256_t {
    uint8_t bytes[32];
    std::string string() const;
};


struct xxh128_payload_t {
    std::filesystem::path path;
    std::array<std::uint8_t, 16> output{}; // 128-bit hash
};

::sigil::yield xxh128_hash(xxh128_payload_t& payload) noexcept;

} // namespace sigil::math
