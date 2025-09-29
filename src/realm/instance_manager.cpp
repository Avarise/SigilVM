#include "thaumaturgy/yield.h"
#include <sigil/realm/instance.h>
#include <thaumaturgy/thaumaturgy.h>

namespace sigil::realm {

::thaumaturgy::yield instance_t::pause() {
    return ::thaumaturgy::yield();
}

::thaumaturgy::yield instance_t::unpause() {
    return ::thaumaturgy::yield();;
}

} // namespace sigil::realm