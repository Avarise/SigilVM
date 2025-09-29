#pragma once

#include <sigil/common.h>
#include <cstdint>
#include <string>
#include <vector>

namespace sigil::realm {

struct instance_t {
    std::string name;
    std::vector<uint64_t> managed_entities;

    ::sigil::yield pause();
    ::sigil::yield unpause();
};

} // namespace sigil::realm
