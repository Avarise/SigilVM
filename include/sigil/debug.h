#pragma once

/**
 * Debug header for SigilVM
 * Provides sigil::dcout (debug conole out), which redirects:
 * in debug build to std::cout
 * in release build to null stream
 */

#include <streambuf>
#include <iostream>
#include <ostream>

namespace sigil {

#ifdef SIGIL_DEBUG

inline std::ostream& dcout = std::cout;

#else // !SIGIL_DEBUG

struct null_streambuf final : public std::streambuf {
    int overflow(int c) override {
        return c;
    }
};

struct null_ostream final : public std::ostream {
    null_streambuf buf;
    null_ostream() : std::ostream(&buf) {}
};

inline null_ostream dcout;

#endif // SIGIL_DEBUG

} // namespace sigil
