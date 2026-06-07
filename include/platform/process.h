#pragma once

#include "../core/status.h"

#include <cstddef>

#if defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #include <windows.h>

    namespace sigilvm::platform {
        using native_process_id_t = DWORD;
    }

#elif defined(__linux__)
    #include <sys/types.h>
    #include <unistd.h>

    namespace sigilvm::platform {
        using native_process_id_t = pid_t;
    }

#else
    #error Unsupported platform
#endif

namespace sigilvm::platform {

struct process_descriptor_t {
    int argc = 0;

    const char **argv = nullptr;
    const char **envp = nullptr;

    native_process_id_t pid = native_process_id_t{};
    bool initialized = false;
};

inline ::sigilvm::status_t process_initialize(
    process_descriptor_t& p,
    int argc,
    const char **argv,
    const char **envp
) {
    ::sigilvm::status_t st;

    if (p.initialized) {
        return st.set_state(::sigilvm::state::fail);
    }

    if (argc < 0) {
        return st.set_state(::sigilvm::state::fail);
    }

    p.argc = argc;
    p.argv = argv;
    p.envp = envp;

#if defined(_WIN32)
    p.pid = ::GetCurrentProcessId();

#elif defined(__linux__)
    p.pid = ::getpid();

#endif

    p.initialized = true;

    return st;
}

} // namespace sigilvm::platform