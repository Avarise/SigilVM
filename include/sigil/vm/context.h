#pragma once

#include <sigil/platform/app.h>

namespace sigil::vm {

/**
 * @brief
 * Primary context for IPC
 */
struct context_t {
    context_t(const platform::app_descriptor_t desc);
    ~context_t();
};

} // namespace sigil::vm
