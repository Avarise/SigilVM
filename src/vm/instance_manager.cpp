#include <sigil/common.h>
#include <sigil/vm/instance.h>
#include <sigil/common.h>

namespace sigil::realm {

::sigil::yield instance_t::pause() {
    return ::sigil::yield();
}

::sigil::yield instance_t::unpause() {
    return ::sigil::yield();;
}

} // namespace sigil::realm