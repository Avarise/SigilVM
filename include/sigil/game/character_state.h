#pragma once
#include <array>
#include <string>
#include <functional>

#include <sigil/game/dice.h>
#include <sigil/game/attribute.h>

namespace sigil::game {

struct attribute_state_t {
    die_t rest;
    int8_t pending_delta;

    die_t current_round_value() const;
    die_t effective_value() const;

    void apply_delta(int delta);
    void commit();
};

struct character_state_t {
    std::string name;

    int rest_level;
    int temp_level;

    std::array<attribute_state_t, (size_t)attribute_t::count> attrs;

    std::string display_name() const;

    void apply_damage(attribute_t attr, int amount);
    void apply_heal(attribute_t attr, int amount);

    void commit_round();

    void recompute_temp_level();
};

// logging injection
struct game_logger_t {
    std::function<void(const std::string&)> info;
    std::function<void(const std::string&)> warn;
    std::function<void(const std::string&)> error;
};

}