#include "modules.h"
#include "assets.h"
#include "env.h"
#include "events.h"
#include "runtime.h"
#include "work.h"
#include <cstdio>

int sigil::modules::get_count(void) {
    return sigil::env::modules_registry.size();
}

sigil::modules::module_t* sigil::modules::peek(int id) {
    int count = sigil::modules::get_count(); 
    if (count < 1) return NULL;
    if (id > count || id < 0) return NULL;

    return sigil::env::modules_registry[id];
}

sigil::modules::module_t* sigil::modules::peek(std::string name) {
    int count = sigil::modules::get_count();
    if (count < 1) return NULL;

    for (int i = 0; i < count; i++) {
        sigil::modules::module_t *temp = (sigil::modules::module_t*)sigil::env::modules_registry[i];
        if (temp == NULL) return NULL;

        std::string *curr_name = &temp->name;
        if (curr_name->compare(name) == 0) return temp;
    }

    return NULL;
}


bool sigil::modules::wq_can_alloc(int id) {
    bool ans = (sigil::env::vm_state == sigil::STARTED) || (sigil::env::vm_state == sigil::BOOTING);
    ans &= (sigil::env::workers_num_current < sigil::env::workers_num_max);
    if (!ans) printf("cannot create worker module with ID:%d\n", id);
    return ans;
}

bool sigil::modules::can_import(const char *name) {
    bool ans = (sigil::env::vm_state != sigil::NOT_STARTED) && (sigil::env::vm_state != sigil::HALTED);
    if (!ans) printf("cannot import module %s\n", name);
    return ans;
}

void sigil::modules::update_signatures() {
    int size = sigil::modules::get_count();
    if (size < 1) return;

    sigil::env::longest_mod_name = 0;

    for (int i = 0; i < size; i++) {
        sigil::modules::module_t *temp = (sigil::modules::peek(i));
        if (!temp) continue;
        int current_size = temp->name.size();
        if (current_size > sigil::env::longest_mod_name) sigil::env::longest_mod_name = current_size;
    }
}

sigil::modules::module_t* sigil::modules::get(std::string name) {
    sigil::modules::module_t *ref = sigil::modules::peek(name);

    if (ref) {
        ref->refcount++;
        return ref;
    }

    return NULL;
}

sigil::modules::module_t* sigil::modules::get(int id) {
    sigil::modules::module_t *ref = sigil::modules::peek(id);

    if (ref) {
        ref->refcount++;
        return ref;
    }

    return NULL;
}

int sigil::modules::release (sigil::modules::module_t *ref) {
    if (!ref) return -EPERM;
    if (ref->refcount > 0) ref->refcount--;
    return 0;
}

int sigil::modules::add(sigil::modules::module_t *mod) {            
    if (modules::peek(mod->name) != NULL) {
        //sigil_trace("sigil::modules", "attempted to add existing module\n");
        return -EPERM;
    }
    
    sigil::env::modules_registry.push_back(mod);
    modules::update_signatures();
    return 0;
}

int sigil::modules::remove(std::string name) {
    sigil::modules::module_t *ref = sigil::modules::peek(name);
    if (!ref) return -EINVAL;
    if (ref->refcount > 0) return -EPERM;

    sigil::env::modules_registry.erase(sigil::env::modules_registry.begin() + ref->id);
    return 0;
}

void sigil::modules::view(const sigil::modules::module_t *target, bool padding) {
    if (target == NULL) return;

    if (padding)    printf("[ %*s {%d} ] -> ", sigil::env::longest_mod_name, target->name.c_str(), target->id);
    else            printf("%s {%d} -> ", target->name.c_str(), target->id);

    printf("type: %s, parent: %s, refcount: %u, data: %p, init/deinit: %p/%p\n",
        sigil::utils::asset_type_tostr(target->type),
        target->parent ? target->parent->name.c_str() : "-none-",
        target->refcount, target->data, target->init, target->deinit);
}

void sigil::modules::view_all() {
    int count = sigil::modules::get_count();
    printf("runtime: %s has %d modules\n", sigil::env::hostname, count);
    if (!count) return;
    
    for (int i = 0; i < count; i++) {
        sigil::modules::module_t *current = sigil::modules::peek(i);
        sigil::modules::view(current, true);
    }
}

sigil::modules::module_t* sigil::modules::import(const char *path, bool wait_for_rt) {
    printf("runtime: insertion from file not yet available\n");
    return 0;
}

sigil::modules::module_t* sigil::modules::import(sigil::modules::static_module_descriptor_t mod_desc, bool wait_for_rt) {
    if (!sigil::modules::can_import(mod_desc.name.c_str())) return NULL;

    //printf("runtime: parsing module: %s\n", mod_desc.name.c_str());
    //sigil_trace("sigil::system","parsing %s\n", mod_desc.name.c_str());

    sigil::modules::module_t *mod = new sigil::modules::module_t;
    mod->name = mod_desc.name;
    mod->type = sigil::ASSET_GENERIC_MODULE;
    mod->id = sigil::modules::get_count();
    mod->refcount = 0;
    mod->parent = sigil::modules::get(0);
    mod->init = mod_desc.init;
    mod->deinit = mod_desc.deinit;
    mod->data = NULL;
    sigil::modules::add(mod);
    
    sigil::work_t init_work;
    init_work.type = WORK_ORDER;
    init_work.work.order = mod->init;
    init_work.ctx = NULL;

    sigil::work_t deinit_work;
    deinit_work.type = WORK_ORDER;
    deinit_work.work.order = mod->deinit;
    deinit_work.ctx = NULL;

    int status;
    if (wait_for_rt) {
        sigil::runtime::attach_responder(sigil::events::peek(0), init_work);
    } else {
        status = mod->init(NULL);
        printf("runtime: running auto init for %s\n", mod->name.c_str());
    }

    sigil::runtime::attach_responder(sigil::events::peek(runtime::SHUTDOWN), deinit_work);

    mod->refcount++;
    return mod;
}