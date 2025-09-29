#pragma once

#include <iostream>
#include <cassert>
#include <cstdint>
#include <ostream>

#define SIGIL_UNUSED(x) (void)(x)

namespace sigil {

struct version_t {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    uint32_t build;
    std::string_view tag;

    constexpr std::string_view to_string() const noexcept;
};

constexpr version_t SIGILVM_VERSION = {1, 0, 0, 1, "pre-alpha"};


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


// SigilVM Yield and Trap
enum class yield_state : std::uint8_t {
    ok = 0,        // successful execution
    partial,       // completed with recoverable issues
    fail,          // failed but execution is still well-defined
    trap           // abnormal termination was captured
};

enum class yield_intent : std::uint8_t {
    none = 0,      // no opinion
    stop,          // do not continue
    retry,         // attempt again
    defer          // postpone execution
};

enum class yield_origin : std::uint8_t {
    local = 0,
    worker,
    process,
    remote
};

struct yield final {
    yield_state  state  = yield_state::ok;
    yield_intent intent = yield_intent::none;
    yield_origin origin = yield_origin::local;

    std::uint32_t code  = 0;   // user / subsystem defined
    std::uint64_t info  = 0;   // opaque auxiliary field

    constexpr yield() noexcept = default;

    constexpr explicit yield(yield_state s) noexcept
        : state(s) {}

    constexpr bool is_ok() const noexcept {
        return state == yield_state::ok;
    }

    constexpr bool is_trap() const noexcept {
        return state == yield_state::trap;
    }

    constexpr bool is_failure() const noexcept {
        return state == yield_state::fail
            || state == yield_state::trap;
    }

    constexpr yield& set_state(yield_state s) noexcept {
        state = s;
        return *this;
    }

    constexpr yield& set_intent(yield_intent i) noexcept {
        intent = i;
        return *this;
    }

    constexpr yield& set_origin(yield_origin o) noexcept {
        origin = o;
        return *this;
    }

    constexpr yield& set_code(std::uint32_t c) noexcept {
        code = c;
        return *this;
    }

    constexpr yield& set_info(std::uint64_t i) noexcept {
        info = i;
        return *this;
    }
};

constexpr inline yield operator|(yield a, const yield& b) noexcept {
    if (static_cast<std::uint8_t>(b.state)
        > static_cast<std::uint8_t>(a.state)) {
        a.state = b.state;
        a.intent = b.intent;
        a.origin = b.origin;
        a.code   = b.code;
        a.info   = b.info;
    }
    return a;
}

constexpr inline yield& operator|=(yield& a, const yield& b) noexcept {
    a = a | b;
    return a;
}

constexpr inline bool operator!(const yield& y) noexcept {
    return y.is_failure();
}

// Exception Trap
template <typename F>
yield contain(yield& out, F&& fn) noexcept {
    try {
        fn();
    } catch (...) {
        out.set_state(yield_state::trap);
    }
    return out;
}



} // namespace sigil
