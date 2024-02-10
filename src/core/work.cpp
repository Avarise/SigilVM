#include <cstdio>
#include "work.h"

void sigil::workqueue_task(void *ctx) {
    workqueue_descriptor_t *data = (workqueue_descriptor_t*)ctx;

    while (true) {
        data->wq_mutex.lock();

        int wq_size = data->wq.size();

        if (wq_size < 1) {
            data->wq_mutex.unlock();
            sigil::utils::delay(0,SIGIL_WQ_IDLE_NS);
            continue;
        }
        
        if (data->wq.front().type != WORK_ORDER) {
            data->wq.pop();
            data->wq_mutex.unlock();
            printf("Invalid work type\n");
            continue;
        }

        sigil::order_t order = data->wq.front().work.order;
        void *context = data->wq.front().ctx;

        data->wq.pop();
        data->items_processed++;
        data->wq_mutex.unlock();

        int status = order(context);

        if (status != 0) {
            printf("Error\n");
        }
    }
}