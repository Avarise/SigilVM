#pragma once

#include <filesystem>
#include <cstdint>

namespace sigil::data {

enum action_kind_t : uint32_t {
    ACTION_SKIP,
    ACTION_COPY,
    ACTION_MOVE,
    ACTION_DELETE,
    ACTION_TRANSFORM
};

enum transform_t : uint32_t {
    TRANSFORM_NONE,
    TRANSFORM_XOR
};

struct action_t {
    action_kind_t kind;
    transform_t   transform;

    std::filesystem::path src;
    std::filesystem::path dst;

    uint64_t flags;
};

} // namespace sigil::data
