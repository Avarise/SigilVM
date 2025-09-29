#pragma once
#include <sigil/platform/app.h>

namespace sigil::realm {

struct context_t {
    context_t(platform::app_descriptor_t dsc);
    ~context_t();
};


} // namespace sigil::realm
