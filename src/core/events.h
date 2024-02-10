#pragma once
#include <string>
#include "assets.h"
#include "work.h"

#define SYS_EVENT_COUNT 8
#define SYS_EVENT_LAST_ID (SYS_EVENT_COUNT - 1)
#define SYS_EVENT_INIT "init"
#define SYS_EVENT_SHUTDOWN "shutdown"

namespace sigil {
    namespace events {
        struct event_data_t {
            asset_t *reporter;
            void *data;
            int data_len;
        };

        struct event_t : asset_t {
            asset_t *owner;
            event_data_t report;
            bool enabled;
            std::string name;
            std::vector<work_t> work;
        };

        int get_count(void);
        event_t* peek(int id);
        event_t* peek(std::string sig_name);
        event_t* get(int id);
        event_t* get(std::string sig_name);
        int add(event_t *sig);
        int remove(std::string sig_name);
        // Returns negative error code or number of responses on success (can be 0)
        int trigger(const event_t *target);
        //inline int attach_responder(sigil::event_t *target, sigil::work_t responder);
        //inline int deattach(sigil::event *target, sigil::event_responder_t callback);
        void view(const event_t *target, bool padding, int i);
        void view_all();
    }
}