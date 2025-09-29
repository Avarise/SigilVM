#include <sigil/game/character_state.h>
#include <algorithm>

namespace sigil::game {

// --- attribute_state_t

die_t attribute_state_t::current_round_value() const
{
    return rest;
}

die_t attribute_state_t::effective_value() const
{
    return die_clamp((int)rest + pending_delta);
}

void attribute_state_t::apply_delta(int delta)
{
    pending_delta += delta;
}

void attribute_state_t::commit()
{
    rest = die_clamp((int)rest + pending_delta);
    pending_delta = 0;
}

// --- character_state_t

std::string character_state_t::display_name() const
{
    return "Lv." + std::to_string(rest_level) + " " + name +
           " (" + std::to_string(temp_level) + ")";
}

void character_state_t::apply_damage(attribute_t attr, int amount)
{
    auto& a = attrs[(size_t)attr];
    a.apply_delta(-amount);
    recompute_temp_level();
}

void character_state_t::apply_heal(attribute_t attr, int amount)
{
    auto& a = attrs[(size_t)attr];
    a.apply_delta(+amount);
    recompute_temp_level();
}

void character_state_t::commit_round()
{
    for (auto& a : attrs)
        a.commit();

    recompute_temp_level();
}

void character_state_t::recompute_temp_level()
{
    int delta_sum = 0;

    for (auto& a : attrs)
        delta_sum += a.pending_delta;

    temp_level = rest_level + delta_sum;
}

}
