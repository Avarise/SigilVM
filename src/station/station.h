#pragma once
/*
    Module header for SigilVM, reponsibilities:
        - WLAN management, listing, connecting
        - Import/export list of WLANs
        - implementing web server
*/
#include "../core/utils.h"
#include "../core/modules.h"
#include "../core/runtime.h"
#include "../core/networking.h"
#include <vector>
#include <map>

#define MOD_STATION "station"

namespace sigil {
    namespace station {
        struct station_data_t {
            std::vector<sigil::network::wifi_entry_t> networks_found;
            std::map<std::string, int> networks_name_map;
            sigil::modules::module_t *scanner_worker;
        };

        extern sigil::modules::module_t *mod;
        extern sigil::station::station_data_t *data;

        int scan_networks();
        void scanner_task(void *context);
        int init(void *ctx);
        int deinit(void *ctx);
        int import();
    }
}