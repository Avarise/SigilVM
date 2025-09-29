#pragma once

/**
 * file: include/sigil/platform/process.h
 * Process header for SigilVM, centered around 
 * sigil::platform::process_descriptor_t
 * Mandatory for use with exec wrappers.
 */
 
#include <sigil/common.h>

namespace sigil::platform {

struct process_descriptor_t {
    int    argc;
    const char **argv;
    const char **envp;
    
    pid_t pid = -1;
    bool initialized = false;
};

inline sigil::yield process_initialize(
    process_descriptor_t& p,
    int argc,
    const char **argv,
    const char **envp
) {
    sigil::yield st;
    
    if (p.initialized) {
        return st.set_state(yield_state::fail);
    }

    p.argc = argc;
    p.argv = argv;
    p.envp = envp;
    p.initialized = true;

    return st;
}

} // namespace sigil::platform