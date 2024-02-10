#include <asm-generic/errno-base.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <signal.h>

#include "runtime.h"
#include "assets.h"
#include "events.h"
#include "modules.h"
#include "work.h"
#include "env.h"

bool sigil::runtime::can_iterate() {
    return sigil::env::vm_state == sigil::STARTED || sigil::env::vm_state == sigil::HALTED;
}

uint32_t sigil::runtime::get_core_count() {
    return sigil::env::hw_cores;
}

sigil::modules::module_t*
sigil::runtime::workthread::create(const char *desc, sigil::modules::module_t *parent) {
    int err = 0;
    
    err = !modules::wq_can_alloc(env::workers_num_current);
    if (err) {
        printf("runtime: cannot create workthread");
        return NULL;
    }

    sigil::modules::module_t *module = new sigil::modules::module_t;
    if (!module) return NULL;

    char module_name[32];
    snprintf(module_name, 32, "wt_%u_%s", sigil::env::workers_num_current, desc);

    sigil::workthread_descriptor_t *data = new workthread_descriptor_t;

    if (!data) {
        delete module;
        return NULL;
    }

    module->name = std::string(module_name);
    module->type = ASSET_WORKTHREAD;
    module->id = sigil::modules::get_count();
    module->refcount = 0;
    module->parent = parent;
    module->init = NULL;
    module->data = data;
    parent->refcount++;
    printf("runtime: created %s\n", module->name.c_str());

    if (sigil::modules::add(module) != 0) {
        delete module;
        delete data;
        parent->refcount--;
        return NULL;
    }

    //sigil::env::workers_num_current++;

    module->refcount++;
    return module;
}

int sigil::runtime::workthread::start(sigil::modules::module_t *wt) {
    if (!wt) return -ENOMEM;
    if (wt->type != ASSET_WORKTHREAD && wt->type != ASSET_WORKQUEUE) return EINVAL;
    sigil::workthread_descriptor_t *data = (sigil::workthread_descriptor_t*)wt->data;

#   ifdef TARGET_LINUX
    data->thread = std::thread(data->task, data->ctx);
#   elif defined (TARGET_FREERTOS)
    xTaskCreate(work.task, wt->name.c_str(), 1024, work.task_data, 10, &data->task_handle);
#   endif
    return 0;
}

int sigil::runtime::workthread::set_workitem(sigil::modules::module_t *wt, sigil::work_t work) {
    if (!wt) return -ENOMEM;

    if (wt->type != ASSET_WORKTHREAD && wt->type != ASSET_WORKQUEUE) return EINVAL;
    if (work.type != WORK_TASK) return EINVAL;

    sigil::workthread_descriptor_t *data = (sigil::workthread_descriptor_t*)wt->data;
    data->task = work.work.task;
    data->ctx = work.ctx;


    return 0;
}

int sigil::runtime::workqueue::release() {
    int err;

    return err;
}

sigil::modules::module_t*
sigil::runtime::workqueue::create(const char *desc, sigil::modules::module_t *parent) {
    if (!sigil::modules::wq_can_alloc(sigil::env::workers_num_current)) return NULL;

    sigil::modules::module_t *wq = new sigil::modules::module_t;
    if (!wq) return NULL;

    char container_name[32];
    snprintf(container_name, 32, "wq_%u_%s", sigil::env::workers_num_current, desc);

    sigil::workqueue_descriptor_t *wq_data = new sigil::workqueue_descriptor_t;
    if (!wq_data) {
        delete wq;
        return NULL;
    }

    wq->name = std::string(container_name);
    wq->type = sigil::ASSET_WORKQUEUE;
    wq->id = sigil::modules::get_count();
    wq->refcount = 0;
    wq->parent = parent;
    wq->init = NULL;
    wq->data = wq_data;
    parent->refcount++;

    wq_data->task = workqueue_task;
    wq_data->ctx = wq_data;
    
    printf("runtime: created %s\n", wq->name.c_str());

    if (sigil::modules::add(wq) != 0) {
        delete wq;
        delete wq_data;
        parent->refcount--;
        return NULL;
    }

    sigil::env::workers_num_current++;

    wq->refcount++;
    return wq;
}

int sigil::runtime::workqueue::start(sigil::modules::module_t *wq) {
    if (!wq) return -ENOMEM;
    if (wq->type != ASSET_WORKQUEUE) return EINVAL;
    sigil::workqueue_descriptor_t *data = (sigil::workqueue_descriptor_t*)wq->data;

#   ifdef TARGET_LINUX
    data->thread = std::thread(data->task, data->ctx);
#   elif defined (TARGET_FREERTOS)
    xTaskCreate(work.task, wt->name.c_str(), 1024, work.task_data, 10, &data->task_handle);
#   endif
    return 0;
}

int sigil::runtime::workqueue::stop(sigil::modules::module_t *wq) {
    int err;

    return err;
}

int sigil::runtime::workqueue::add_work(sigil::modules::module_t *wq, sigil::work_t work) {
    if (!wq) return -ENOMEM;

    sigil::workqueue_descriptor_t *wq_data = (sigil::workqueue_descriptor_t*)wq->data;
    wq_data->wq_mutex.lock();
    wq_data->wq.push(work);
    wq_data->wq_mutex.unlock();
    return 0;
}
int sigil::runtime::spawn_runtime_workqueue() {
    size_t num_wqs = sigil::env::runtime_workers.size();
    if (num_wqs >= 8) { 
        return -ENOMEM;
    }
    sigil::modules::module_t *rtwq = sigil::runtime::workqueue::create("runtime", sigil::modules::peek(0));
    if (!rtwq) return -ENOMEM;

    sigil::env::runtime_workers.push_back(rtwq);
    sigil::runtime::workqueue::start(rtwq);
    return 0;
}

int sigil::runtime::schedule_work_to_runtime_workqueue(sigil::work_t work) {
    uint32_t size = sigil::env::runtime_workers.size();
    if (size == 0) return -EINVAL;

    uint32_t worker_idx = sigil::env::work_processed++ % size;
    sigil::modules::module_t *worker = sigil::env::runtime_workers[worker_idx];

    return sigil::runtime::workqueue::add_work(worker, work);
} 

int sigil::runtime::exec(const char *cmd) {
    return 0;
}

int sigil::runtime::iteratere() {
    int err = 0;
    sigil::utils::delay(sigil::env::iter_forced_ms);
    sigil::env::iters++;
    return err;
}

int sigil::runtime::create_system_events() {
    printf("runtime: creating system events -> ");

    sigil::modules::module_t *system = sigil::modules::peek(0);

    for (int i = 0; i < SYS_EVENT_COUNT; i++) {
        sigil::events::event_t *temp = new sigil::events::event_t;

        if (temp == NULL) {
            printf("fail on id %d\n", i);
            return -ENOMEM;
        }

        temp->name = "reserved";
        temp->type = ASSET_EVENT;
        temp->work = {};
        temp->enabled = false;
        temp->owner = system;
        temp->refcount = 0;
        temp->report.reporter = system;
        temp->report.data = NULL;
        temp->report.data_len = 0;
        sigil::events::add(temp);
    }

    printf("success!\n");
    return 0;
}

int sigil::runtime::prepare_events() {
    int err = sigil::runtime::create_system_events();
    if (err) return err;

    sigil::env::events_registry[INIT]->name = SYS_EVENT_INIT;
    sigil::env::events_registry[INIT]->enabled = true;
    sigil::env::events_registry[SHUTDOWN]->name = SYS_EVENT_SHUTDOWN;
    sigil::env::events_registry[SHUTDOWN]->enabled = true;
    return 0;
}

int rt_self_init(void *ctx) {
    sigil::events::peek(0)->enabled = false;
    return 0;
}

int sigil::runtime::attach_responder(sigil::events::event_t *target, sigil::work_t responder) {
    if (target == NULL) {
        printf("No target to attach event responder\n");
        return -EINVAL;
    }

    //printf("Responder %p attached to %s\n", responder.work.order, target->name.c_str());
    target->work.push_back(responder);
    return 0;
}

void signal_handler(int s) {
    printf("runtime: got signal %d\n", s);
    sigil::events::event_t *shutdown = sigil::events::peek(sigil::runtime::SHUTDOWN);
    sigil::events::trigger(shutdown);
    exit(1);
}

int sigil::runtime::import() {
    int err = 0;
    sigil::env::iters = 0;
    sigil::env::iter_forced_ms = 0;
    sigil::env::workers_num_max = MAX_THREADS;
    sigil::env::workers_num_current = 0;
    sigil::env::longest_mod_name = 0;
    sigil::env::longest_event_name = 0;
    sigil::env::work_processed = 0;
    sigil::env::vm_state = sigil::BOOTING;
#ifdef TARGET_LINUX
    std::srand(time(NULL));
#endif
    printf("runtime: attemting import -> ");

    sigil::modules::module_t *mod_system = new sigil::modules::module_t;
    if (!mod_system) {
        printf("failed...\n");
        return -ENOMEM;
    } 

    mod_system->name = MOD_SYSTEM;
    mod_system->type = sigil::ASSET_SYSTEM;
    mod_system->id = 0;
    mod_system->refcount = 0;
    mod_system->parent = NULL;
    mod_system->init = rt_self_init;
    mod_system->deinit = NULL;
    mod_system->data = NULL;
    printf("success!\n");
    err = sigil::modules::add(mod_system);

    sigil::env::hw_cores = std::thread::hardware_concurrency();
    printf("runtime: detected CPU cores:%u\n", sigil::env::hw_cores);

    sigil::runtime::prepare_events();

    sigil::work_t responder_work;
    responder_work.work.order = mod_system->init;
    responder_work.ctx = NULL;
    responder_work.type = sigil::WORK_ORDER;

    sigil::runtime::attach_responder(sigil::events::peek(0), responder_work);

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
    printf("runtime: import complete(%d)\n", sigil::env::vm_state);
    return err;
    }

/*
    TODO: get rid of character match parsing
*/
int sigil::runtime::parse_cstr(const char *payload, int size) {
    if (size > 1) {
        if (payload[0] == '/') {
            printf("File preview not yet available\n");
        } else if (payload[0] == '#') {
            int heapsize = 0;
#ifdef TARGET_8266
            heapsize = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
#endif
            //mod_print(sigil::modules::peek(0), "available heap: %u\n", heapsize);
        } else if (strcmp(payload, "exit") == 0) {
            exit(0);
        } else if (strcmp(payload, "lsmod") == 0) {
            sigil::modules::view_all();
        } else if (strcmp(payload, "lsevents") == 0) {
            sigil::events::view_all();
        } else {
            printf("Error parsing: %s\n", payload);
        }
    }
    return 0;
}

int sigil::runtime::start() {
    if (sigil::env::vm_state != sigil::BOOTING) {
        printf("Cannot start, system not imported\n");
        return -EPERM;
    }

    sigil::events::event_t *init_event = sigil::events::peek(0);
    sigil::modules::module_t *sys = sigil::modules::peek(0);

    if (!init_event || !sys) {
        return -EPERM;
    }

    sigil::env::vm_state = sigil::STARTED;

    int num_modules = sigil::modules::get_count();
    int num_boot_responses = sigil::events::trigger(init_event);

    printf("runtime: boot finished, %d/%d modules responded on init\n", num_boot_responses, num_modules);
    return 0;
}

void sigil::runtime::shutdown() {
    sigil::modules::module_t *system = sigil::modules::peek(0);
    //mod_print(system, "shut down started\n");
    printf("runtime: shutdown started\n");

    sigil::events::event_t *shutdown = sigil::events::get(sigil::runtime::SHUTDOWN);
    shutdown->report.reporter = system;

    sigil::events::trigger(shutdown);
}

void sigil::runtime::restart() {
    for (int i = 5; i >= 0; i--) {
        //mod_print(modules::peek(0), "restarting in %d seconds...\n", i);
        printf("runtime: restarting in %d seconds...\n", i);
        sigil::utils::delay(1000);
    }
    printf("runtime: restarting now.\n");
    sigil::runtime::shutdown();
    fflush(stdout);
#   ifndef __linux__
    esp_restart();
#   endif
}