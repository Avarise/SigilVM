#pragma once
#include <filesystem>
#include <unistd.h>
#include "../core/status.h"

namespace sigilvm::fs {

struct file_info_t {
    std::filesystem::path path;
    uint64_t size;
    uint64_t mtime;

    bool is_regular;
    bool is_symlink;
};

struct file_handler_t {
    std::filesystem::path path;
    size_t file_size;
    uint8_t *data;

    file_handler_t(const std::filesystem::path path);
    ~file_handler_t();

    // Allows to save a file to a new path
    ::sigilvm::status_t save_to(const std::filesystem::path path);
};


    
::sigilvm::status_t copy_file_tree
(std::filesystem::path &src, std::filesystem::path &dst, bool ignore_symlinks = true);

bool files_are_identical
(const std::filesystem::path &a, const std::filesystem::path &b);

bool binary_exists_in_path(const char* name);

} // namespace ::sigil::fs
