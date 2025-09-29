#pragma once

#include <cstdint>

#define SIGIL_BIT(n) (1u << (n))

namespace sigil {

/**
 * @brief 
 * Status codes for return based error checking
 */
enum status_t : uint32_t {
    // --- [0] Neutral / Success Domain --------------------------------------
    VM_OK                         = 0u,                     // Everything succeeded
    VM_PARTIAL_SUCCESS            = SIGIL_BIT(0),           // Some operations succeeded, some failed
    VM_COMPLETED_WITH_WARNINGS    = SIGIL_BIT(1),           // Completed but returned non-fatal notices

    // --- [1] Runtime State Domain ------------------------------------------
    VM_IDLE                       = SIGIL_BIT(2),           // System is idle
    VM_RUNNING                    = SIGIL_BIT(3),           // Active execution state
    VM_INITIALIZING               = SIGIL_BIT(4),           // During startup / allocation
    VM_RESTARTING                 = SIGIL_BIT(5),           // Restart requested
    VM_SHUTTING_DOWN              = SIGIL_BIT(6),           // Graceful shutdown in progress
    VM_TERMINATED                 = SIGIL_BIT(7),           // Fully stopped

    // --- [2] Operational Errors --------------------------------------------
    VM_FAILED                     = SIGIL_BIT(8),          // General failure
    VM_TIMEOUT                    = SIGIL_BIT(9),          // Operation timed out
    VM_INTERRUPTED                = SIGIL_BIT(10),         // Interrupted externally (signal/user)
    VM_ABORTED                    = SIGIL_BIT(11),         // Forcibly stopped
    VM_INVALID_STATE              = SIGIL_BIT(12),         // Invalid call in current VM state
    VM_CORRUPTED                  = SIGIL_BIT(13),         // Corrupted internal data

    // --- [3] Memory / Resource ---------------------------------------------
    VM_OUT_OF_MEMORY              = SIGIL_BIT(14),          // Insufficient memory
    VM_RESOURCE_MISSING           = SIGIL_BIT(15),          // Missing asset, file, or dependency
    VM_RESOURCE_LOCKED            = SIGIL_BIT(16),          // Resource in use or locked
    VM_ALREADY_EXISTS             = SIGIL_BIT(17),          // Duplicate creation attempt
    VM_HANDLE_INVALID             = SIGIL_BIT(18),          // Invalid or stale handle/context

    // --- [4] Argument / Input ----------------------------------------------
    VM_ARG_NULL                   = SIGIL_BIT(19),          // Null pointer passed
    VM_ARG_INVALID                = SIGIL_BIT(20),          // Argument out of valid range
    VM_FORMAT_INVALID             = SIGIL_BIT(21),          // Invalid data format or header

    // --- [5] System / IO ---------------------------------------------------
    VM_IO_ERROR                   = SIGIL_BIT(22),          // File, socket, or device error
    VM_PERMISSION_DENIED          = SIGIL_BIT(23),          // Access denied
    VM_NOT_SUPPORTED              = SIGIL_BIT(24),          // Not supported on this platform/config
    VM_SYSTEM_SHUTDOWN            = SIGIL_BIT(25),          // Host system shutting down
    VM_DEPENDENCY_MISSING         = SIGIL_BIT(26),          // Library or module not loaded

    // --- [6] Control / Meta ------------------------------------------------
    VM_SKIPPED                    = SIGIL_BIT(27),          // Operation deliberately skipped
    VM_DEFERRED                   = SIGIL_BIT(28),          // Operation scheduled later
    VM_RETRYABLE                  = SIGIL_BIT(29),          // Operation can be retried safely

    // --- [7] Reserved for Future Extensions --------------------------------
    VM_RESERVED                   = SIGIL_BIT(30),          // Reserved internal use
    VM_CUSTOM                     = SIGIL_BIT(31),          // User-defined / subsystem-specific
};

inline constexpr status_t operator|(status_t a, status_t b) noexcept {
    return static_cast<status_t>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
    );
}

inline constexpr status_t operator&(status_t a, status_t b) noexcept {
    return static_cast<status_t>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b)
    );
}

inline constexpr status_t operator^(status_t a, status_t b) noexcept {
    return static_cast<status_t>(
        static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b)
    );
}

inline constexpr status_t& operator|=(status_t& a, status_t b) noexcept {
    a = a | b;
    return a;
}

inline constexpr status_t& operator&=(status_t& a, status_t b) noexcept {
    a = a & b;
    return a;
}

inline constexpr status_t& operator^=(status_t& a, status_t b) noexcept {
    a = a ^ b;
    return a;
}

} // namespace sigil
