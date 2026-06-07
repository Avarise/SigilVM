#pragma once

#include <cstdint>
#include <new>
#include <stdexcept>

namespace sigilvm {
    // SigilVM Status and Trap
    enum class state : std::uint8_t {
        ok = 0,        // successful execution
        partial,       // completed with recoverable issues
        fail,          // failed but execution is still well-defined
        trap           // abnormal termination was captured
    };
    
    enum class intent : std::uint8_t {
        none = 0,      // no opinion
        stop,          // do not continue
        retry,         // attempt again
        defer          // postpone execution
    };
    
    enum class origin : std::uint8_t {
        local = 0,
        worker,
        process,
        remote
    };

    enum class trap : std::uint16_t {
        none = 0,
    
        unknown,
    
        std_exception,
    
        bad_alloc,
        bad_cast,
        bad_typeid,
        bad_optional_access,
    
        logic_error,
        invalid_argument,
        domain_error,
        length_error,
        out_of_range,
    
        runtime_error,
        overflow,
        underflow,
        range_error,
    
        system_error,
    
        foreign_exception
    };
    
    struct status_t {
        std::uint32_t code = 0;
        std::uint64_t info = 0;
    
        ::sigilvm::state  state  = state::ok;
        ::sigilvm::intent intent = intent::none;
        ::sigilvm::origin origin = origin::local;
        ::sigilvm::trap trap = trap::none;
        
        constexpr status_t() noexcept = default;

        constexpr explicit status_t(::sigilvm::state s) noexcept
            : state(s) {}
    
        constexpr bool is_ok() const noexcept {
            return state == ::sigilvm::state::ok;
        }
    
        constexpr bool is_trap() const noexcept {
            return state == ::sigilvm::state::trap;
        }
    
        constexpr bool is_failure() const noexcept {
            return state == ::sigilvm::state::fail
                || state == ::sigilvm::state::trap;
        }
    
        constexpr status_t& set_state(::sigilvm::state s) noexcept {
            state = s;
            return *this;
        }
    
        constexpr status_t& set_intent(::sigilvm::intent i) noexcept {
            intent = i;
            return *this;
        }

        constexpr status_t& set_trap(::sigilvm::trap t) noexcept {
            trap = t;
            return *this;
        }
    
        constexpr status_t& set_origin(::sigilvm::origin o) noexcept {
            origin = o;
            return *this;
        }
    
        constexpr status_t& set_code(std::uint32_t c) noexcept {
            code = c;
            return *this;
        }
    
        constexpr status_t& set_info(std::uint64_t i) noexcept {
            info = i;
            return *this;
        }
    };

    template <typename F>
    inline ::sigilvm::status_t contain(F&& fn) noexcept {
        status_t st;
        try {
            fn();
            return st;
        }
        catch (const std::bad_alloc&) {
            return st.set_state(::sigilvm::state::trap).set_trap(::sigilvm::trap::bad_alloc);
        }
        catch (const std::out_of_range&) {
            return st.set_state(::sigilvm::state::trap).set_trap(::sigilvm::trap::out_of_range);
        }
        catch (const std::logic_error&) {
            return st.set_state(::sigilvm::state::trap).set_trap(::sigilvm::trap::logic_error);
        }
        catch (const std::runtime_error&) {
            return st.set_state(::sigilvm::state::trap).set_trap(::sigilvm::trap::runtime_error);
        }
        catch (const std::exception&) {
            return st.set_state(::sigilvm::state::trap).set_trap(::sigilvm::trap::std_exception);
        }
        catch (...) {
            return st.set_state(::sigilvm::state::trap).set_trap(::sigilvm::trap::foreign_exception);
        }
    }
    
    constexpr inline ::sigilvm::status_t operator|(::sigilvm::status_t a, const ::sigilvm:: status_t& b) noexcept {
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
} // namespace sigilvm
