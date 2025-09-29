#pragma once

#include <thaumaturgy/thaumaturgy.h>
#include <filesystem>

namespace sigil::data {

using map_fn_t = ::thaumaturgy::yield(*)(
    const std::filesystem::path &src,
    std::filesystem::path &dst,
    void *user
);

struct mapper_t {
    map_fn_t fn;
    void    *user;
};

}
