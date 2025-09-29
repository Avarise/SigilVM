#pragma once

#include <streambuf>
#include <iostream>
#include <ostream>

namespace sigil
{

#ifdef SIGIL_DEBUG

/* -------------------------------------------------------------
 * DEBUG build
 * dcout is a direct alias to std::cerr (or cout if you prefer)
 * ------------------------------------------------------------- */

inline std::ostream& dcout = std::cout;

#else

/* -------------------------------------------------------------
 * RELEASE build
 * Null stream: swallows everything at compile-time
 * ------------------------------------------------------------- */

struct null_streambuf final : public std::streambuf
{
    int overflow(int c) override
    {
        return c;
    }
};

struct null_ostream final : public std::ostream
{
    null_streambuf buf;

    null_ostream() : std::ostream(&buf) {}
};

inline null_ostream dcout;

#endif

} // namespace sigil
