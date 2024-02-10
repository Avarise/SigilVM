#include "station.h"
#include <cstdio>

sigil::modules::module_t *sigil::station::mod = NULL;
sigil::station::station_data_t *sigil::station::data = NULL;


int sigil::station::scan_networks() {
    int num_found_networks = 0;
    // TODO: Implement scan, async not needed
    return num_found_networks;
}

void sigil::station::scanner_task(void *context) {
    while (true) {
        utils::delay(1000);
        //mod_print(station::mod, "running network scan...\n");
        scan_networks();
    }
#   ifdef TARGET_8266
    vTaskDelete(NULL);
#   endif
}

int sigil::station::init(void *ctx) {
    if (!mod) return -EPERM;

    sigil::station::data = new station_data_t;

    if (!sigil::station::data) return -ENOMEM;

    sigil::station::data->scanner_worker = sigil::runtime::workthread::create("station_scanner", station::mod);
    if (!sigil::station::data->scanner_worker) {
        //mod_print(station::mod, "error - could not spawn worker module\n");
        return -ENOMEM;
    }

    mod->data = data;

    return 0;
}

int sigil::station::deinit(void *ctx) {
    sigil::modules::release(station::mod);
    return 0;
}

int sigil::station::import() {
    modules::static_module_descriptor_t import_desc;

    printf(MOD_STATION": attempting to import...\n");

    import_desc.name = MOD_STATION;
    import_desc.init = sigil::station::init;
    import_desc.deinit = sigil::station::deinit;

    sigil::station::mod = sigil::modules::import(import_desc, false);

    if (!sigil::station::mod) {
        printf("station: import failed...\n");
        return -ENOMEM;
    }

    printf("station: import success!\n");
    return 0;
}
