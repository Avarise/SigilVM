#pragma once
#include <vector>
#include <string>

#include <sigil/game/character_state.h>

namespace sigil::game {

enum class entry_type_t {
    character,
    custom
};

struct initiative_entry_t {
    int initiative;
    entry_type_t type;

    character_state_t* character;
    std::string label;

    std::string display() const;
};

struct initiative_tracker_t {
    std::vector<initiative_entry_t> entries;

    int current_index;
    int round;

    game_logger_t* logger;

    void sort_desc();

    initiative_entry_t* current();

    void next_turn();

    void commit_round();

    void add_character(character_state_t* c, int init);
    void add_custom(const std::string& label, int init);
};

}
