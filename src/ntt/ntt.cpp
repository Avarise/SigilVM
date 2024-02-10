#include "ntt.h"

sigil::modules::module_t *sigil::ntt::mod = NULL;
sigil::ntt::ntt_manager_t *sigil::ntt::prv = NULL;


sigil::ntt::scene_t* sigil::ntt::get_default_scene(void) {
    if (sigil::ntt::prv->scenes.size() < 1) return NULL;

    sigil::ntt::prv->scenes[0]->refcount++;
    return sigil::ntt::prv->scenes[0];
}

int sigil::ntt::register_engine(sigil::task_t engine_task, engine_data_t *engine_data) {
    sigil::work_t engine_work;
    engine_work.type =  WORK_TASK;
    engine_work.work.task = engine_task;
    engine_work.ctx = engine_data;

    sigil::modules::module_t *wt = runtime::workthread::create(engine_data->name.c_str(), ntt::mod);
    sigil::runtime::workthread::set_workitem(wt, engine_work);
    sigil::runtime::workthread::start(wt);
    return 0;
}

void sigil::ntt::sync_engine(ntt::engine_data_t *target_engine) {
    if (target_engine == NULL) return;

    std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;
    long int target_frame_us = 0;

    timestamp = std::chrono::high_resolution_clock::now();

    if (target_engine->target_fps > 0) {
        // Converting FPS into microsecond frame time
        target_frame_us = 1000000 / target_engine->target_fps;
        target_engine->delta_us = std::chrono::duration_cast<std::chrono::microseconds>
                                    (timestamp - target_engine->last_frame_end_tp).count();
        if (target_engine->delta_us < target_frame_us) {
            utils::microdelay(target_frame_us - target_engine->delta_us);
            timestamp = std::chrono::high_resolution_clock::now();
            target_engine->delta_us = std::chrono::duration_cast<std::chrono::microseconds>
                                        (timestamp - target_engine->last_frame_end_tp).count();
        }
    }
    
    else if (target_engine->target_fps == 0) {
        target_engine->delta_us = std::chrono::duration_cast<std::chrono::microseconds>
                                    (timestamp - target_engine->last_frame_end_tp).count();
    }

    target_engine->last_frame_end_tp = timestamp;
}

int sigil::ntt::init(void *ctx) {
    if (!mod) return -EPERM;
    printf("ntt: initializing\n");

    sigil::ntt::prv = new ntt_manager_t;

    if (!sigil::ntt::prv) return -ENOMEM;

    sigil::ntt::scene_t *default_scene = new sigil::ntt::scene_t;

    prv->scenes.push_back(default_scene);

    mod->data = prv;
    prv->last_engine_id = 0;

    return 0;
}

int sigil::ntt::deinit(void *ctx) {
    return 0;
}

int sigil::ntt::import() {
    modules::static_module_descriptor_t import_desc;
    printf("ntt-ecs: attempting to import -> ");

    import_desc.name = "ntt-ecs";
    import_desc.init = sigil::ntt::init;
    import_desc.deinit = sigil::ntt::deinit;

    sigil::ntt::mod = sigil::modules::import(import_desc, true);

    if (!sigil::ntt::mod) {
        printf("failed...\n");
        return -ENOMEM;
    }

    printf("success!\n");
    return 0;
}