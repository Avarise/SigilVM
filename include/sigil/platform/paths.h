#pragma once

#include <string>

namespace sigil::platform {
    
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
    
}