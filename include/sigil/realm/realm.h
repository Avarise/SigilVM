#pragma once

#include "sigil/status.h"
#include <cstdint>
#include <string>
#include <vector>

namespace sigil {
    struct world_instance_t {
        std::string inst_name;
        std::vector<uint64_t> managed_entities;

        status_t pause();
        status_t unpause();
    };
}