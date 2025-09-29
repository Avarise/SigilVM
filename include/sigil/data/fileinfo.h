#pragma once

#include <filesystem>
#include <cstdint>

namespace sigil::data {

struct file_info_t {
    std::filesystem::path path;
    uint64_t size;
    uint64_t mtime;

    bool is_regular;
    bool is_symlink;
};

}
