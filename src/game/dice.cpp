#include <sigil/game/dice.h>
#include <algorithm>

namespace sigil::game {

const char* die_to_string(die_t d)
{
    switch (d) {
        case die_t::d4: return "d4";
        case die_t::d6: return "d6";
        case die_t::d8: return "d8";
        case die_t::d10: return "d10";
        case die_t::d12: return "d12";
        case die_t::d20: return "d20";
        case die_t::d100: return "d100";
    }
    return "?";
}

die_t die_clamp(int value)
{
    value = std::clamp(value, 0, (int)die_t::d100);
    return (die_t)value;
}

}