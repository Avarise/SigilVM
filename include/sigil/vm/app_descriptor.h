#pragma once

namespace sigil::vm {

/**
 * @brief
 * Contains identity of a process, used to create contexts for SigilVM modules.
 */
struct app_descriptor_t {
    // ----- identity -----
    const char* app_name;      // short executable name
    const char* app_id;        // unique app identifier (optional)

    // ----- process info -----
    int argc;                  // raw argc from main()
    const char** argv;         // raw argv from main()

    // ----- filesystem roots -----
    const char* runtime_dir;   // $XDG_RUNTIME_DIR/sigilvm/<app> or similar
    const char* config_dir;    // $XDG_CONFIG_HOME/sigilvm/<app>

    // ----- dbus naming -----
    const char* dbus_prefix;   // "org.sigilvm.<app>"

    bool debug = false;
    bool verbose = false;
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