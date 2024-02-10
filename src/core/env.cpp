#include "env.h"

namespace sigil {
    namespace env {
        char hostname[32] = "SigilVM";
        uint8_t debug_mode = 0;
        long unsigned int iters = 0;
        uint32_t iter_forced_ms = 0;
        uint32_t hw_cores = 0;
        sigil::vm_state_t vm_state = NOT_STARTED;
        uint8_t workers_num_max = 0;
        uint8_t workers_num_current = 0;
        uint8_t longest_mod_name = 0; // update at insertion time, use this for formatting
        //extern uint8_t longest_signal_name; // update at insertion time, use this for formatting
        uint8_t longest_event_name = 0; // update at insertion time, use this for formatting
        std::vector<sigil::modules::module_t*> runtime_workers = {};
        std::vector<sigil::modules::module_t*> modules_registry = {};
        std::vector<sigil::events::event_t*> events_registry = {};
        uint32_t work_processed = 0; // Number of work items executed by runtime workqueues
        bool fullscreen = false;
        bool limit_fps = false;
        bool vsync = true;
    }
}