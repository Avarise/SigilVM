#pragma once

#include <sigil/vm/app_descriptor.h>

namespace sigil::vm {

/**
 * @brief
 * Primary context for IPC
 */
struct context_t {
    context_t(const app_descriptor_t desc);
    ~context_t();
};

} // namespace sigil::vm
