#pragma once

#include <sigil/status.h>
#include <cstdint>
#include <string>
#include <vector>

namespace sigil::realm {

struct instance_t {
    std::string name;
    std::vector<uint64_t> managed_entities;

    status_t pause();
    status_t unpause();
};

} // namespace sigil::realm
