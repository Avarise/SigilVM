#pragma once

#include <string_view>
#include "status.h"

namespace sigilvm {

struct version_t {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    uint32_t build;
    std::string_view tag;

    constexpr std::string_view to_string() const noexcept;
};

constexpr version_t SIGILVM_VERSION = {0, 2, 0, 1, "pre-alpha"};
    
}