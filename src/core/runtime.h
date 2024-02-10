#pragma once
#include <chrono>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <thread>
#include "events.h"
#include "modules.h"
#include "utils.h"
#include "memory.h"

#define MOD_SYSTEM "runtime"

namespace sigil {
    namespace runtime {
        enum sys_events_t {
            INIT = 0,
            SHUTDOWN = SYS_EVENT_LAST_ID,
        };

        namespace workthread {
            sigil::modules::module_t *get();
            sigil::modules::module_t* create(const char *desc, sigil::modules::module_t *parent);
            int release();
            int remove();
            int start(sigil::modules::module_t *wt);
            int stop(sigil::modules::module_t *wt);
            int set_workitem(sigil::modules::module_t *wt, sigil::work_t work);
        }

        namespace workqueue {        
            sigil::modules::module_t *get();
            sigil::modules::module_t *create(const char *desc, sigil::modules::module_t *parent);
            int release();
            int remove();
            int start(sigil::modules::module_t *wq);
            int stop(sigil::modules::module_t *wq);
            int add_work(sigil::modules::module_t *wq, sigil::work_t work);
        }

        //sigil::modules::module_t* create_workthread(const char *desc, sigil::modules::module_t *parent);
        //int set_workthread(sigil::modules::module_t *wt, sigil::work_t work);


        //sigil::modules::module_t *get_workqueue();
        //sigil::modules::module_t *create_workqueue(const char *desc, sigil::modules::module_t *parent);
        //int release_workqueue();
        //int remove_workqueue();
        //int start_workqueue(sigil::modules::module_t*wq);
        //int stop_workqueue(sigil::modules::module_t *wq);
        //int push_work_to_workqueue(sigil::modules::module_t *wq, sigil::work_t work);

        struct sync_data_t {
            uint64_t iters;
            // Render rate in HZ, 0 for unlimited
            uint32_t target_render_rate;
            double delta_us; // 10e-6 second delta time
            std::chrono::time_point<std::chrono::high_resolution_clock> ts_render_end;
        };


        inline void sync(sync_data_t *target_data) {
            if (target_data == NULL) return;
            target_data->iters++;

            std::chrono::time_point<std::chrono::high_resolution_clock> ts_now = std::chrono::high_resolution_clock::now();

            if (!target_data->target_render_rate) {
                target_data->delta_us = (double)std::chrono::duration_cast<std::chrono::microseconds>(ts_now - target_data->ts_render_end).count();
                target_data->ts_render_end = ts_now;
                return;
            }

            double microsecond_frametime_target = 10e5f / target_data->target_render_rate;

            sigil::utils::microdelay(microsecond_frametime_target);


            ts_now = std::chrono::high_resolution_clock::now();
            target_data->delta_us = (double)std::chrono::duration_cast<std::chrono::microseconds>(ts_now - target_data->ts_render_end).count();
            target_data->ts_render_end = ts_now;
        }

        // Spawn is a create and start operation
        int spawn_runtime_workqueue();
        int schedule_work_to_runtime_workqueue(sigil::work_t work);

        bool can_iterate();
        uint32_t get_core_count();
        int exec(const char *cmd);
        int iteratere();
        int create_system_events();
        int prepare_events();
        int attach_responder(sigil::events::event_t *target, sigil::work_t responder);
        //int deattach_responder(sigil::event *target, sigil::event_responder_t callback);
        int import();
        int parse_cstr(const char *payload, int size);
        int start();
        void run_watchdog();
        void run_watchdog_threaded();
        void shutdown();
        void restart();
    }
}