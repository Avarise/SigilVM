#pragma once

#include <string_view>
#include <cstdint>

namespace sigil {

struct version_t {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    uint32_t build;
    std::string_view tag;

    constexpr std::string_view to_string() const noexcept;
};

constexpr version_t SIGILVM_VERSION = {1, 0, 0, 1, "pre-alpha"};

} // namespace sigil
