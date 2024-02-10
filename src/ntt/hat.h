#pragma once
/*
    Hat is a tool to spawn large amounts of rabbits inside a scene
*/

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <chrono>

#include "ntt.h"
#include "general-components.h"
#include "graphic-components.h"
#include "game-components.h"
#include "../core/memory.h"
#include "../core/env.h"

#define GENERATION_BLOCK_SIZE (512 * 4) 

namespace sigil {
    namespace hat_engine {
        extern std::atomic_uint32_t blocks_created;

        struct generator_block {
            uint32_t block_id;
            sigil::ntt::scene_t *scene;
        };

        /*
            Some notes on mass entity generation:
            Per entity approach does not benefit form multithreading because 
            spawner threads still have to block before accessing the component store.
            Solution would be to move ECS-like generation, and instead of generating
            whole entity at once, we split generation into Mesh Generation, Name Generation
            and pass whole generation process instead to thread instead of block of 
            entities we want to spawn.
        */
        inline void create_one_entity(ntt::entity_t id, ntt::scene_t *scene) {
            scene->asset_mtx.lock();
            scene->entities.push_back(id);
            scene->asset_mtx.unlock();
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


            sigil::ntt::add_name_component(id, std::string(npc_name));
            sigil::ntt::add_static_mesh_component(id, sigil::asset_t("./assets/meshes/test.msh"));
            sigil::ntt::add_texture_component(id);
        }

        inline void batch_generate_entities(ntt::entity_t first_id, ntt::entity_t last_id, ntt::scene_t *scene) {
            for (uint32_t id = first_id; id <= last_id; id++) create_one_entity(id, scene);
        }

        inline int generate_entity_block(void *context) {
            auto ts1 = std::chrono::high_resolution_clock::now();
            uint32_t block_id = ((generator_block*)context)->block_id;
            sigil::ntt::scene_t *scene = ((generator_block*)context)->scene;

            sigil::ntt::entity_t block_end_offset = GENERATION_BLOCK_SIZE - 1;
            sigil::ntt::entity_t scene_size_offset = scene->entities.size();

            sigil::ntt::entity_t first_id = scene_size_offset + block_id * GENERATION_BLOCK_SIZE;
            sigil::ntt::entity_t last_id = first_id + block_end_offset; 

            batch_generate_entities(first_id, last_id, scene);

            sigil::hat_engine::blocks_created++;

            uint32_t wc = blocks_created;

            free(context);
            auto ts2 = std::chrono::high_resolution_clock::now();
            long int duration = std::chrono::duration_cast<std::chrono::microseconds>(ts2 - ts1).count();
            long int dura_ms = duration / 1000;
            long int dura_us = duration % 1000;
            if (sigil::env::debug_mode) {
                printf("Hat: Generated %d sized block in %lu.%lums [%d %s total]\n", 
                    GENERATION_BLOCK_SIZE, dura_ms, dura_us, wc, wc == 1 ? "block" : "blocks");
            }
            return 0;
        }

        inline void pull_rabbits(sigil::ntt::scene_t *scene, uint32_t num_rabbits) {
            auto ts1 = std::chrono::high_resolution_clock::now();
            sigil::memory::memstat_t memstats;
            uint32_t num_blocks = num_rabbits / GENERATION_BLOCK_SIZE;
            uint32_t remainder = num_rabbits % GENERATION_BLOCK_SIZE;
            sigil::hat_engine::blocks_created = 0;

            if (sigil::env::debug_mode) {
                printf("Hat: scheduled creation of %u entities\n", num_rabbits);
                if (num_blocks) printf("Hat: preparing %u blocks\n", num_blocks);
            }


            for (uint32_t i = 0; i < num_blocks; i++) {
                //printf("Ordered block %u\n", i);
                generator_block *block = new generator_block;
                block->block_id = i;
                block->scene = scene;

                sigil::work_t work;
                work.type = WORK_ORDER;
                work.work.order = generate_entity_block;
                work.ctx = block;
                sigil::runtime::schedule_work_to_runtime_workqueue(work);
            }

            // while(blocks_created != num_blocks) {
            //     sigil::utils::delay(5);
            // }
            
            if (remainder) {
                ntt::entity_t first_id = blocks_created * GENERATION_BLOCK_SIZE;
                ntt::entity_t last_id = (blocks_created * GENERATION_BLOCK_SIZE) + remainder - 1;

                if (sigil::env::debug_mode) {
                    printf("Hat: %u rabbits generated in last block [ID:%u to %u] \n", remainder, first_id, last_id);
                }

                for (ntt::entity_t id = first_id; id <= last_id; id++) {
                    create_one_entity(id, scene);
                }
            }

            auto ts2 = std::chrono::high_resolution_clock::now();

            long int duration = std::chrono::duration_cast<std::chrono::microseconds>(ts2 - ts1).count();
            long int dura_ms = duration / 1000;
            long int dura_us = duration % 1000;

            sigil::memory::get_memstats(&memstats);
        }

    }
}