#pragma once
#include <cstdint>
#include "events.h"
#include "modules.h"

namespace sigil {
    enum vm_state_t {
        NOT_STARTED = 0,
        BOOTING = 1,
        STARTED = 2,
        HALTED = 4,
    };

    namespace env {
        extern char hostname[32];
        extern uint8_t debug_mode;
        extern long unsigned int iters;
        extern uint32_t iter_forced_ms;
        extern uint32_t hw_cores;
        extern sigil::vm_state_t vm_state;
        extern uint8_t workers_num_max;
        extern uint8_t workers_num_current;
        extern uint8_t longest_mod_name; // update at insertion time, use this for formatting
        //extern uint8_t longest_signal_name; // update at insertion time, use this for formatting
        extern uint8_t longest_event_name; // update at insertion time, use this for formatting
        extern std::vector<sigil::modules::module_t*> runtime_workers;
        extern std::vector<sigil::modules::module_t*> modules_registry;
        extern std::vector<sigil::events::event_t*> events_registry;
        extern uint32_t work_processed; // Number of work items executed by runtime workqueues
        extern bool fullscreen;
        extern bool limit_fps;
        extern bool vsync;
    }
}