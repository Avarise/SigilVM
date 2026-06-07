#pragma once

/**
 * SigilVM Header for execution of system binaries.
 * Centered around Process Execution Unit, which provides
 * options and structures to choose what to do with process
 * I/O files, storage of return code, and exports + argument passing.
 */

#include <unordered_map>
#include <sys/types.h>
#include <unistd.h>
#include <optional>
#include <cstdint>
#include <string>
#include <vector>

#include "../core/status.h"


namespace sigilvm::runtime {

    // Exec modes for PEU
    enum exec_mode_t : uint32_t {
        EXEC_REPLACE = 0,   // execve / execvp, does not return on success
        EXEC_WAIT,          // fork + exec + waitpid
        EXEC_DETACH,        // fork + setsid + exec, no wait
    };
    
    // STDIO redirection for interprocess
    enum stdio_mode_t : uint32_t {
        STDIO_INHERIT = 0,  // inherit parent's stdio
        STDIO_NULL,         // redirect to /dev/null
        STDIO_CAPTURE,      // capture via memfd_create
    };
    
    struct proc_exec_result_t {
        pid_t pid = -1;
    
        // valid only for EXEC_WAIT
        int   exit_code = -1;     // normalized exit code
        int   term_signal = 0;    // terminating signal, if any
    
        // valid only if STDIO_CAPTURE
        int   stdout_fd = -1;     // memfd
        int   stderr_fd = -1;     // memfd
    };
    
    
    struct proc_exec_unit_t {
        std::string target = {};                  // executable path or name
        std::vector<std::string> args = {};        // argv[1..]
        std::vector<std::string> exports = {};     // KEY=VALUE pairs
        std::string workdir = {};                  // empty = inherit cwd
        exec_mode_t  exec_mode  = EXEC_WAIT;
        stdio_mode_t stdin_mode = STDIO_INHERIT;
        stdio_mode_t stdout_mode = STDIO_INHERIT;
        stdio_mode_t stderr_mode = STDIO_INHERIT;
        proc_exec_result_t result = {};
        bool use_path = true;               // execvp vs execve
    
    
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
        
        void clean() {
            this->target = {};
            this->exports = {};
            this->args = {};
        }

        ::sigilvm::status_t run();
    };

} // namespace sigilvm::runtime
