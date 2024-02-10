#pragma once
/*
    Header for ntt module of SigilVM.
    ntt is work in progress of a general purpose ecs
    General purpose comes as support for multiple types of components and scenes:
        - regular 3d scene with camera
        - data processing scene with data endpoints and scripted entities
*/

#include <sys/types.h>
#include <cstdint>
#include <chrono>
#include <cstdio>
#include <vector>
#include <map>
#include "../core/assets.h"
#include "../core/utils.h"
#include "../core/modules.h"
#include "../core/runtime.h"

namespace sigil {
    namespace ntt {
        typedef uint32_t entity_t;

        typedef struct scene_t : sigil::asset_t {
            std::vector<sigil::ntt::entity_t> entities;
            bool paused;
        } scene_t;

        typedef struct {
            std::vector<scene_t*> scenes;
            uint32_t last_engine_id;
        } ntt_manager_t;

        struct engine_data_t : sigil::runtime::sync_data_t {
            std::string name;
            sigil::ntt::scene_t *target_scene;
            int target_fps; // in HZ
            std::chrono::time_point<std::chrono::high_resolution_clock> last_frame_end_tp;
            long int delta_us; // microseconds delta time
            void *custom_data;
            uint32_t iters;
        };

        extern sigil::modules::module_t *mod;
        extern sigil::ntt::ntt_manager_t *prv;

        sigil::ntt::scene_t* get_default_scene(void);

        int destroy_entity(entity_t id);

        int register_engine(sigil::task_t engine_task, engine_data_t *engine_data);

        /*
            Call this after each engine run, this delays engine enough to reach
            target FPS. Even with no target, still has to be called to get delta time
        */
        void sync_engine(ntt::engine_data_t *target_engine);

        int init(void *ctx);

        int deinit(void *ctx);
        
        int import();
    }
}