#include <sigil/game/initiative.h>
#include <algorithm>

namespace sigil::game {

std::string initiative_entry_t::display() const
{
    if (type == entry_type_t::character)
        return character->display_name();

    return label;
}

void initiative_tracker_t::sort_desc()
{
    std::sort(entries.begin(), entries.end(),
        [](const auto& a, const auto& b) {
            return a.initiative > b.initiative;
        });
}

initiative_entry_t* initiative_tracker_t::current()
{
    if (entries.empty()) return nullptr;
    return &entries[current_index];
}

void initiative_tracker_t::next_turn()
{
    if (entries.empty()) return;

    int prev = entries[current_index].initiative;

    current_index = (current_index + 1) % entries.size();

    int next = entries[current_index].initiative;

    if (next > prev) {
        round++;
        commit_round();

        if (logger && logger->info)
            logger->info("New round: " + std::to_string(round));
    }
}

void initiative_tracker_t::commit_round()
{
    for (auto& e : entries) {
        if (e.type == entry_type_t::character)
            e.character->commit_round();
    }
}

void initiative_tracker_t::add_character(character_state_t* c, int init)
{
    entries.push_back({init, entry_type_t::character, c, ""});
}

void initiative_tracker_t::add_custom(const std::string& label, int init)
{
    entries.push_back({init, entry_type_t::custom, nullptr, label});
}

}
