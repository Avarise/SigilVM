#pragma once
#include <sigil/vm/app_descriptor.h>

namespace sigil::realm {

struct context_t {
    context_t(vm::app_descriptor_t dsc);
    ~context_t();
};


} // namespace sigil::realm
