#include <sigil/platform/process.h>

namespace sigil::platform {
    
process_descriptor_t::process_descriptor_t
(const int argc, const char **argv, const char **envp) : argc(argc), argv(argv), envp(envp) {}
    
}