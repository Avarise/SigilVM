#include <sigil/realm/instance.h>
#include <sigil/status.h>

namespace sigil::realm {

status_t instance_t::pause() {
    return VM_OK;
}

status_t instance_t::unpause() {
    return VM_OK;
}

} // namespace sigil::realm