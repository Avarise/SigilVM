#include <sigil/platform/process.h>
#include <sigil/platform/app.h>
#include <cstring>


// Name extraction helper
static const char*
extract_app_name(const char* path) {
    if (!path) return "";
    const char* slash = std::strrchr(path, '/');
    return (slash ? slash + 1 : path);
}

namespace sigil::platform {

app_descriptor_t::app_descriptor_t(process_descriptor_t p) : process(p) {
    this -> app_name = extract_app_name((process.argc > 0) ? process.argv[0] : "SIGILVM_INVALID_APP_NAME");
}


} // namespace sigil
