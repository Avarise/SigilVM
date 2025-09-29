#pragma once

/**
 * Application header for SigilVM, centered around sigil::platform::app_descriptor_t
 * This structure is mandatory for applications that want to use SigilVM headers,
 * especially parts that revolve around subsystem contexts.
 */

#include <sigil/platform/process.h>
 
namespace sigil::platform {

struct app_descriptor_t {
    const char *app_id;
    const char *app_name;
    const char *dbus_prefix;
    process_descriptor_t process;
    app_descriptor_t(process_descriptor_t p);
};

} // namespace sigil::platform