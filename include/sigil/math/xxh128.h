#pragma once

#include <array>
#include <filesystem>
#include <cstdint>

#include <thaumaturgy/yield.h>

namespace sigil::crypto {

struct xxh128_payload_t {
    std::filesystem::path path;
    std::array<std::uint8_t, 16> output{}; // 128-bit hash
};

::thaumaturgy::yield xxh128_hash(xxh128_payload_t& payload) noexcept;

} // namespace sigil::crypto
