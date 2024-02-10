#pragma once
#include "assets.h"
#include "utils.h"
#include "work.h"
#include "events.h"
#include <cstdio>
#include <mutex>
#include <queue>
#ifdef TARGET_LINUX
#include <thread>
#endif

namespace sigil {
    namespace modules {
        struct static_module_descriptor_t {
            std::string name;
            sigil::order_t init;
            sigil::order_t deinit;
        };

        struct module_t : asset_t {
            uint32_t id;
            std::string name;
            module_t *parent;
            sigil::order_t init;
            sigil::order_t deinit;
        };

        struct device_descriptor_t {

        };

        int get_count(void);
        bool wq_can_alloc(int id);
        bool can_import(const char *name);
        void update_signatures();
        module_t* peek(int id);
        module_t* peek(std::string name);
        module_t* get(std::string name);
        module_t* get(int id);
        // Imported modules are registered onto vector, with their init deinit
        // functions. You can autoinit them, or wait for runtime to init.
        module_t* import(const char *path, bool wait_for_rt);
        module_t* import(static_module_descriptor_t mod_desc, bool wait_for_rt);
        int add(module_t *mod);
        int release (module_t *ref);
        int remove(std::string name);
        void view(const module_t *target, bool padding);
        void view_all();
        std::string serialize_all();
    } /* end of namespace modules */
}