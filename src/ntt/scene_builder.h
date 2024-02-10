#pragma once
/*
    ntt engine responsible for generating and importing scenes
*/
#include "ntt.h"

namespace sigil {
    namespace scene_builder {
        int import_from_file(const char *path) {

            return 0;
        }

        void randomizer_mesh_batch(){}
        
        void randomizer_name_batch() {
            char npc_name[32];

            switch (id % 8) {
                case 0:
                    snprintf(npc_name, 32, "Jimmy #%u", id);
                    break;
                case 1:
                    snprintf(npc_name, 32, "Bobby #%u", id);
                    break;
                case 2:
                    snprintf(npc_name, 32, "Lenny #%u", id);
                    break;
                case 3:
                    snprintf(npc_name, 32, "Kenny #%u", id);
                    break;
                case 4:
                    snprintf(npc_name, 32, "Dim Goon #%u", id);
                    break;
                default:
                    snprintf(npc_name, 32, "Test Dummy #%u", id);
                    break;
            }
        }

        int create_random_entities(ntt::scene_t *scene, ntt::entity_t first_id, ntt::entity_t last_id) {

            return 0;
        }
    }
}