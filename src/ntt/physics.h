#pragma once
/*
    Realm is an engine that provides simple physics.
*/
#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "ntt.h"
#include "graphic-components.h"
#include "general-components.h"
#include "game-components.h"
#include "../core/memory.h"
#include "../core/math.h"

#define PAUSED_ENGINE_DELAY_MS 250

namespace sigil {
    namespace physics {
        static sigil::ntt::engine_data_t data;

        static struct top_speed {
            float speed;
            ntt::entity_t record_breaker;
        } top_speed;

        struct realm_engine_data_t {
            float top_speed;
            ntt::entity_t top_speed_record_breaker;
        };

        void push_in_random_direction(sigil::ntt::entity_t id) {
            sigil::ntt::dynamic_mesh_component_t *component = ntt::entity_to_dynamic_mesh_component(id);
            uint32_t xunit = sigil::math::get_ru32(0, 255);
            uint32_t yunit = sigil::math::get_ru32(0, 255);
            uint32_t zunit = sigil::math::get_ru32(0, 255);

            xunit % 2 ? component->transform.translation.x += 0.01f * xunit
                      : component->transform.translation.x -= 0.01f * xunit;
            yunit % 2 ? component->transform.translation.y += 0.01f * yunit
                      : component->transform.translation.y -= 0.01f * yunit;
            zunit % 2 ? component->transform.translation.z += 0.01f * zunit
                      : component->transform.translation.z -= 0.01f * zunit;
        }

        // void update_top_speed(sigil::ntt::entity_t id, sigil::ntt::transform_component_t *transform) {
        //     float entity_speed = sigil::math::v3_len(transform->translation);
        //     if (entity_speed > top_speed.speed) {
        //         top_speed.speed = entity_speed;
        //         top_speed.record_breaker = id;
        //         printf("Top entity speed: %f by %u\n", top_speed.speed, top_speed.record_breaker);
        //     }
        // }

        void apply_3dtransforms(sigil::ntt::scene_t *target_scene) {
            size_t num_items = ntt::dynamic_mesh_component_store.objects.size();
            ntt::dynamic_mesh_component_t *mesh;

            for (size_t i = 0; i < num_items; i++) {
                mesh = ntt::get_dynamic_mesh_component(i);
                mesh->position.x += mesh->transform.translation.x * data.delta_us;
                mesh->position.y += mesh->transform.translation.y * data.delta_us;
                mesh->position.z += mesh->transform.translation.z * data.delta_us;
            }
        }

        void run_once(sigil::ntt::scene_t *target_scene) {
            apply_3dtransforms(target_scene);
        }

        void engine_task(void *context) {
            sigil::ntt::engine_data_t *data = (sigil::ntt::engine_data_t*)context; 

            while (true) {
                if (data->target_scene->paused) {
                    sigil::utils::delay(PAUSED_ENGINE_DELAY_MS);
                    continue;
                }

                physics::run_once(data->target_scene);
                ntt::sync_engine(data);
            }
        }

        int start_engine(sigil::ntt::scene_t *target_scene, uint32_t tick_rate) {
            int err = 0;
            sigil::physics::data.target_fps = tick_rate;
            sigil::physics::data.target_scene = target_scene;
            sigil::physics::data.last_frame_end_tp = std::chrono::high_resolution_clock::now();
            sigil::physics::data.name = "realm-engine";
            err = sigil::ntt::register_engine(engine_task, &sigil::physics::data);
            return err;
        }
    }
}