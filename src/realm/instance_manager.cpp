#include "sigil/status.h"
#include "sigil/realm/realm.h"

namespace sigil {

status_t world_instance_t::pause() {
    return VM_OK;
}

status_t world_instance_t::unpause() {
    return VM_OK;
}

} // namespace sigil