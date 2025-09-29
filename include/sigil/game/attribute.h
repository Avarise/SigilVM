#pragma once
#include <cstdint>

namespace sigil::game {

enum class attribute_t : uint8_t {
    str = 0,
    dex,
    con,
    int_,
    sen,
    aur,
    count
};

inline const char* attribute_to_string(attribute_t a) {
    switch (a) {
        case sigil::game::attribute_t::str: return "Strength";
        case sigil::game::attribute_t::dex: return "Dexterity";
        case sigil::game::attribute_t::con: return "Constitution";
        case sigil::game::attribute_t::int_: return "Intellect";
        case sigil::game::attribute_t::sen: return "Senses";
        case sigil::game::attribute_t::aur: return "Aura";
        default: return "Missing";
    }
}

}
