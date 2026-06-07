#pragma once

#include <ostream>

#define SIGIL_UNUSED(x) (void)(x)

namespace sigilvm {

// dcout - compile time determined output stream
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
    
}