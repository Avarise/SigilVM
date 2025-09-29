#pragma once
#include <cstdint>
#include <string>

namespace sigil::game {

enum class die_t : uint8_t {
    d4 = 0,
    d6,
    d8,
    d10,
    d12,
    d20,
    d100,
};

const char* die_to_string(die_t d);
die_t die_clamp(int value);

} // namespace sigil::game