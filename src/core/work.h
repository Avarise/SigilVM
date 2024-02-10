#pragma once
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include "target.h"
#include "utils.h"

#define SIGIL_WQ_IDLE_NS 50

namespace sigil {
    // Task is expected to run continuously and not return
    typedef void (*task_t)(void*);

    // Order returns status of execution
    typedef int (*order_t)(void*);

    enum work_type_t {
        WORK_UNKNOWN,
        WORK_TASK,
        WORK_ORDER,
    };

    union work_call_t {
        task_t task;
        order_t order;
    };

    struct work_t {
        work_type_t type;
        work_call_t work;
        void *ctx;
    };

    struct workthread_descriptor_t {
#       if defined(TARGET_LINUX)
        std::thread thread;
#       elif defined(TARGET_FREERTOS)
        TaskHandle_t thread;
#       endif
        sigil::task_t task;
        void *ctx;
    };

    struct workqueue_descriptor_t : workthread_descriptor_t {
        std::queue<sigil::work_t> wq;
        std::mutex wq_mutex;
        uint64_t items_processed = 0;
    };

    void workqueue_task(void *ctx);
}