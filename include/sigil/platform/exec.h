#pragma once

#include <sigil/status.h>
#include <string>
#include <vector>
#include <cstdint>

#include <unistd.h>     // pid_t
#include <sys/types.h>  // pid_t

namespace sigil::platform {

//
// ------------------------------------------------------------
// Execution modes
// ------------------------------------------------------------
//

enum exec_mode_t : uint32_t {
    EXEC_REPLACE = 0,   // execve / execvp, does not return on success
    EXEC_WAIT,          // fork + exec + waitpid
    EXEC_DETACH,        // fork + setsid + exec, no wait
};


//
// ------------------------------------------------------------
// stdio handling
// ------------------------------------------------------------
//

enum stdio_mode_t : uint32_t {
    STDIO_INHERIT = 0,  // inherit parent's stdio
    STDIO_NULL,        // redirect to /dev/null
    STDIO_CAPTURE,     // capture via memfd_create
};


//
// ------------------------------------------------------------
// Process execution result
// ------------------------------------------------------------
//

struct proc_exec_result_t {
    pid_t pid = -1;

    // valid only for EXEC_WAIT
    int   exit_code = -1;     // normalized exit code
    int   term_signal = 0;    // terminating signal, if any

    // valid only if STDIO_CAPTURE
    int   stdout_fd = -1;     // memfd
    int   stderr_fd = -1;     // memfd
};


//
// ------------------------------------------------------------
// Process Execution Unit (PEU)
// ------------------------------------------------------------
//

struct proc_exec_unit_t {
    //
    // target
    //
    std::string target = {};                  // executable path or name
    bool        use_path = true;               // execvp vs execve

    //
    // arguments
    //
    std::vector<std::string> args = {};        // argv[1..]

    //
    // environment
    //
    std::vector<std::string> exports = {};     // KEY=VALUE pairs

    //
    // execution context
    //
    std::string workdir = {};                  // empty = inherit cwd

    //
    // modes
    //
    exec_mode_t  exec_mode  = EXEC_WAIT;
    stdio_mode_t stdin_mode = STDIO_INHERIT;
    stdio_mode_t stdout_mode = STDIO_INHERIT;
    stdio_mode_t stderr_mode = STDIO_INHERIT;

    //
    // results (filled by execute)
    //
    proc_exec_result_t result = {};

    //
    // --------------------------------------------------------
    // fluent helpers
    // --------------------------------------------------------
    //

    proc_exec_unit_t& set_target(const std::string& path) {
        target = path;
        return *this;
    }

    proc_exec_unit_t& push_argument(const std::string& arg) {
        args.push_back(arg);
        return *this;
    }

    proc_exec_unit_t& export_var(const std::string& key,
                                 const std::string& value) {
        exports.push_back(key + "=" + value);
        return *this;
    }

    proc_exec_unit_t& set_workdir(const std::string& path) {
        workdir = path;
        return *this;
    }

    proc_exec_unit_t& set_exec_mode(exec_mode_t m) {
        exec_mode = m;
        return *this;
    }

    proc_exec_unit_t& set_stdio_mode(stdio_mode_t in,
                                     stdio_mode_t out,
                                     stdio_mode_t err) {
        stdin_mode  = in;
        stdout_mode = out;
        stderr_mode = err;
        return *this;
    }
};


//
// ------------------------------------------------------------
// Execution entry point
// ------------------------------------------------------------
//
// Returns:
//   0        on success
//   -errno  on failure
//
// For EXEC_REPLACE:
//   success does not return
//

status_t execute(proc_exec_unit_t& peu);

} // namespace sigil::platform
