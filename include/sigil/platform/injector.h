#pragma once

#include <sys/types.h>
#include <cstdint>
#include <vector>
#include <string>

namespace sigil::platform {

enum class inspect_mode_t {
    live,        // /proc/<pid>/mem
    controlled   // ptrace attach
};

struct mem_region_t {
    uintptr_t start = 0;
    uintptr_t end   = 0;

    bool readable = false;
    bool writable = false;

    std::string name;
};

struct process_inspector_t {
    pid_t pid = -1;
    inspect_mode_t mode = inspect_mode_t::live;

    int mem_fd = -1;         // for /proc/<pid>/mem
    bool attached = false;   // ptrace state

    std::vector<mem_region_t> regions;

    bool initialized = false;
};

} // namespace sigil::platform