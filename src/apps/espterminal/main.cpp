#include "../../core/modules.h"
#include "../../core/runtime.h"
#include "../../station/station.h"
#include "../../hid/hid.h"
#include "../../core/env.h"

inline int app() {
    int err = sigil::runtime::import();
    if (err) return err;

    err = sigil::station::import();
    if (err) return err;

    err = sigil::hid_driver::import();
    if (err) exit(err);

    err = sigil::runtime::start();
    if (err) return err;

    sigil::modules::view_all();
    sigil::events::view_all();

    while (sigil::env::vm_state == sigil::STARTED || sigil::env::vm_state == sigil::HALTED) {
        err = sigil::runtime::iteratere();
        if (err) return err;
    }

    return err;
}

#ifdef __cplusplus
extern "C"{
#endif
void app_main() {
    app();
}
#ifdef __cplusplus
}
#endif  