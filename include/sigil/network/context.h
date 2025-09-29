#pragma once

#include <sigil/vm/app_descriptor.h>

namespace sigil::network {

struct context_t {
    context_t(vm::app_descriptor_t dsc);
    ~context_t();
};

} // sigil::network