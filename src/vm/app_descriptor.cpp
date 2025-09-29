#include <sigil/vm/app_descriptor.h>
#include <cstring>
#include <cstdlib>

namespace sigil::vm {

static const char*
extract_app_name(const char* path) {
    if (!path) return "";
    const char* slash = std::strrchr(path, '/');
    return (slash ? slash + 1 : path);
}

app_descriptor_t
mk_app_descriptor(const int argc, const char** argv) {
    app_descriptor_t out{};

    // ----- identity -----
    out.app_name = extract_app_name((argc > 0) ? argv[0] : "");
    out.app_id   = nullptr;

    // ----- process info -----
    out.argc = argc;
    out.argv = argv;

    // ----- filesystem roots -----
    const char* xdg_runtime = std::getenv("XDG_RUNTIME_DIR");
    const char* xdg_config  = std::getenv("XDG_CONFIG_HOME");

    out.runtime_dir = xdg_runtime;
    out.config_dir  = xdg_config;

    // ----- dbus naming -----
    out.dbus_prefix = nullptr;

    return out;
}

} // namespace sigil
