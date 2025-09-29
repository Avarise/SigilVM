#pragma once
#include <string>
#include <cstdlib>

namespace sigil {

// -------------------------------------------------------------
//  Compile-time SIMD feature detection
// -------------------------------------------------------------

#if defined(__AVX2__)
    static constexpr bool HAS_AVX2 = true;
#else
    static constexpr bool HAS_AVX2 = false;
#endif

// Public getter, resolved at compile time and constexpr-foldable
inline constexpr bool has_avx2_extensions() {
    return HAS_AVX2;
}

// -------------------------------------------------------------
//  Theme directories
// -------------------------------------------------------------

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

} // namespace sigil


// static const fs::path sigil_theme_dir = "/usr/share/sigilvm/themes";
// static const fs::path user_config_dir = fs::path(std::getenv("HOME")) / ".config";