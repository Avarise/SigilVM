#pragma once
#include "thaumaturgy/yield.h"
#include <thaumaturgy/thaumaturgy.h>
#include <filesystem>
#include <unistd.h>

namespace sigil::filesystem {

/**
    * @brief
    * RAII based file handler, that exposes contents as raw, heap allocated chunk of memory.
    */
struct file_handler_t {
    std::filesystem::path path;
    size_t file_size;
    uint8_t *data;

    file_handler_t(const std::filesystem::path path);
    ~file_handler_t();

    // Allows to save a file to a new path
    ::thaumaturgy::yield save_to(const std::filesystem::path path);
};

bool binary_exists_in_path(const char* name);



inline std::string theme_dir_system() {
    return "/usr/share/sigilvm/themes";
}

inline std::string theme_dir_local() {
    const char* home = std::getenv("HOME");
    if (!home)
        return "/tmp/sigilvm/themes";   // minimal fallback

    std::string p;
    p.reserve(256);
    p += home;
    p += "/.local/share/sigilvm/themes";
    return p;
}

inline std::filesystem::path get_home_path() {
    const char* home = std::getenv("HOME");
    if (home && home[0] != '\0') {
        return std::filesystem::path(home);
    }

    // --- Fallback path ---
    const char* user = std::getenv("USER");
    char uid_buf[32];

    if (!user || user[0] == '\0') {
        std::snprintf(uid_buf, sizeof(uid_buf), "uid_%d", getuid());
        user = uid_buf;
    }

    std::filesystem::path fallback =
        std::filesystem::path("/tmp/home") / user;

    // Best-effort creation, ignore failure
    std::error_code ec;
    std::filesystem::create_directories(fallback, ec);

    return fallback;
}


inline std::filesystem::path get_sigilvm_local_path() {
    return get_home_path() / ".local" / "share" / "sigilvm";
}

::thaumaturgy::yield copy_tree(std::filesystem::path &src, std::filesystem::path &dst);

bool files_are_identical(const std::filesystem::path &a, const std::filesystem::path &b);

} // namespace sigil::filesystem
