#pragma once
#include "ntt.h"
#include "../core/math.h"
#include <unordered_map>

namespace sigil {
    namespace ntt {
        /* Name component */
        struct name_component_t {
            std::string name;

            name_component_t() {
                name = "-noname-";
            }

            name_component_t(std::string n) {
                name = n;
            }
        };

        extern struct name_component_container_t {
            std::vector<name_component_t> objects;
            std::unordered_map<ntt::entity_t, uint32_t> entity_map;
            std::mutex mtx;
        } name_component_store;

        int add_name_component(ntt::entity_t id, std::string name);
        name_component_t* get_name_component(uint32_t offset);
        name_component_t* entity_to_name_component(ntt::entity_t id);

        /* Script component */
        struct script_component_t {
            std::string script;
        };

        extern struct script_component_container_t {
            std::vector<script_component_t> objects;
            std::unordered_map<ntt::entity_t, uint32_t> entity_map;
            std::mutex mtx;
        } script_component_store;

        int add_script_component(ntt::entity_t id);
        script_component_t* get_script_component(uint32_t offset);
        script_component_t* entity_to_script_component(ntt::entity_t id);

        /* Controller component */
        struct controller_component_t {
        };

        extern struct controller_component_t_container_t {
            std::vector<controller_component_t> objects;
            std::unordered_map<ntt::entity_t, uint32_t> entity_map;
            std::mutex mtx;
        } controller_component_store;

        int add_controller_component(ntt::entity_t id);
        controller_component_t* get_controller_component(uint32_t offset);
        controller_component_t* entity_to_controller_component(ntt::entity_t id);
    }
}