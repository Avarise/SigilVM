#pragma once
#include "ntt.h"
#include "../core/math.h"
#include <unordered_map>
namespace sigil {
    namespace ntt {
        struct pawn_component_t {
            int hit_points;
            int main_attack_param;
            int normal_defense;
            int special_defense;
            void *custom_data;
        };

        extern struct pawn_component_container_t {
            std::vector<pawn_component_t> objects;
            std::unordered_map<ntt::entity_t, uint32_t> entity_map;
            std::mutex mtx;
        } pawn_component_store;

        int add_pawn_component(ntt::entity_t id);
        pawn_component_t* get_pawn_component(uint32_t offset);
        pawn_component_t* entity_to_pawn_component(ntt::entity_t id);
    }
}