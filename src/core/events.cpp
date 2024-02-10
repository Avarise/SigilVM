#include "events.h"
#include "env.h"
#include "work.h"
#include <cstdio>

int sigil::events::get_count() {
    return sigil::env::events_registry.size();
}

sigil::events::event_t* sigil::events::peek(int id) {
    int count = sigil::events::get_count();

    if (count < 1) return NULL;
    if (id > count) return NULL;

    return sigil::env::events_registry[id];
}

sigil::events::event_t* sigil::events::peek(std::string name)  {
    int count = sigil::env::events_registry.size();

    if (count == 0) return NULL;

    for (int i = 0; i < count; i++) {
        sigil::events::event_t *temp = sigil::env::events_registry[i];

        if (temp->name.compare(name) == 0) return temp;
    }

    return NULL;
}

sigil::events::event_t* sigil::events::get(int id) {
    sigil::events::event_t *target = peek(id);

    if (!target) return NULL;

    target->refcount++;
    return target;
}

sigil::events::event_t* sigil::events::get(std::string sig_name) {
    sigil::events::event_t *target = peek(sig_name);

    if (!target) return NULL;

    target->refcount++;
    return target;
}


int sigil::events::add(event_t *event) {
    if (!event) return -EINVAL;

    int len = event->name.size();
    if (len > sigil::env::longest_event_name) sigil::env::longest_event_name = len;

    sigil::env::events_registry.push_back(event);

    return 0;
}

// Returns negative error code or number of responses on success (can be 0)
int sigil::events::trigger(const sigil::events::event_t *target) {
    if (!target) return -EINVAL;
    if (!target->enabled) return -EPERM;

    int num_trackers = (int)target->work.size();  
    if (num_trackers == 0) return 0;
    
    int num_responses = 0;

    for (int i = 0; i < num_trackers; i++) {
        sigil::work_t current = target->work[i];

        if (current.type != WORK_ORDER) return -EINVAL;

        int status = current.work.order(current.ctx);

        if (status == 0) num_responses++;
    }

    return num_responses;
}

void sigil::events::view(const sigil::events::event_t *target, bool padding, int i) {
    if (target == NULL) return;

    if (padding)    printf("[ %*s {%d} ] -> ", sigil::env::longest_event_name, target->name.c_str(), i);
    else            printf("%s -> ", target->name.c_str());

    printf("owner: %s, callbacks: %d, enabled: %s\n",
            target->owner ? ((sigil::modules::module_t*)(target->owner))->name.c_str() : "-none-",
            (int)target->work.size(),
            target->enabled ?  "yes" : "no");
}

void sigil::events::view_all() {
    int count = sigil::events::get_count();
    printf("runtime: %s has %d events\n", sigil::env::hostname, count);
    if (count == 0) return;
    
    for (int i = 0; i < count; i++) {
        sigil::events::event_t *current = sigil::events::peek(i);
        sigil::events::view(current, true, i);
    }
}