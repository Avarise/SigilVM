#pragma once

namespace sigil::vm {

/**
 * @brief
 * Contains identity of a process, used to create contexts for SigilVM modules.
 */
struct app_descriptor_t {
    const char* dbus_prefix;   // "org.sigilvm.<app>"
    const char* app_name;      // short executable name
    const char* app_id;        // unique app identifier (optional)

    bool verbose = false;
    bool debug = false;
};

/**
 * @brief
 * Creates app_descriptor_t for a process
 * @param argc
 * @param argv
 * @return app_descriptor_t
 */
app_descriptor_t mk_app_descriptor(const int argc, const char** argv);

} // namespace sigil::vm
