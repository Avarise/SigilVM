#pragma once

/**
 * Application header for SigilVM, centered around sigil::platform::app_descriptor_t
 * This structure is mandatory for applications that want to use SigilVM headers,
 * especially parts that revolve around subsystem contexts.
 */

#include <cstdio>
#include <functional>
#include <sigil/platform/process.h>
#include <sigil/common.h>
#include <cstring>
#include <unistd.h>

static const char*
extract_app_name(const char* path) {
    if (!path) return "";
    const char* slash = std::strrchr(path, '/');
    return (slash ? slash + 1 : path);
}

namespace sigil::platform {

inline void default_log_info(const std::string &msg) {
    std::cout << msg << std::endl;
}

inline void default_log_warn(const std::string &msg) {
    std::cout << msg << std::endl;
}

inline void default_log_error(const std::string &msg) {
    fprintf(stderr, "%s\n", msg.c_str());
}

inline void default_log_debug(const std::string &msg) {
    sigil::dcout << msg << std::endl;
}
    
struct app_descriptor_t {
    const char *app_id;
    const char *app_name;
    const char *dbus_prefix;
    bool initialized = false;
    process_descriptor_t process;
    
    std::function<void(const std::string&)> log_info;
    std::function<void(const std::string&)> log_warn;
    std::function<void(const std::string&)> log_error;
    std::function<void(const std::string&)> log_debug;

};

inline sigil::yield app_initialize(
    app_descriptor_t& app,
    const process_descriptor_t& process
) {
    sigil::yield st;
    
    if (app.initialized) {
        return st.set_state(yield_state::fail);
    }

    if (!process.initialized) {
        return st.set_state(yield_state::fail);
    }

    app.process = process; // shallow copy (safe: no ownership)
    app.app_name = extract_app_name((process.argc > 0) ? process.argv[0] : "SIGILVM_INVALID_APP_NAME");

    app.initialized = true;

    return st;
}

} // namespace sigil::platform