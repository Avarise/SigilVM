#pragma once

#include "thaumaturgy/yield.h"
#include <thaumaturgy/thaumaturgy.h>
#include <cstdint>
#include <string>
#include <vector>

namespace sigil::realm {

struct instance_t {
    std::string name;
    std::vector<uint64_t> managed_entities;

    ::thaumaturgy::yield pause();
    ::thaumaturgy::yield unpause();
};

} // namespace sigil::realm
