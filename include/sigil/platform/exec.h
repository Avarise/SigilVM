#pragma once

/**
 * SigilVM Header for execution of system binaries.
 * Centered around Process Execution Unit, which provides
 * options and structures to choose what to do with process
 * I/O files, storage of return code, and exports + argument passing.
 */

#include "thaumaturgy/yield.h"
#include <thaumaturgy/thaumaturgy.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>


namespace sigil::platform {

struct cmd_handler_args {
    std::vector<std::string> args;
    std::vector<std::string> switches;
};
    
using command_handler_t = ::thaumaturgy::yield (*)(const cmd_handler_args);

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
    

/**
 * @brief
 * User-facing structure used to register new command
 */
struct command_t {
    std::vector<std::string> words;     // command tokens: {"theme","set"}
    command_handler_t handler;          // nullptr = invalid
    bool passthrough;                   // allow remaining args to be passed

    command_t(const std::initializer_list<std::string> &w, bool pass, command_handler_t h) noexcept
        : words(w), handler(h), passthrough(pass) {}
};

/**
    * @brief
    * Structure for creation of internal command tree
    */
struct command_node_t {
    std::unordered_map<std::string, command_node_t*> children;
    command_handler_t handler;
    bool passthrough;

    command_node_t() : handler(nullptr), passthrough(false) {}
};

/*
    * @brief
    * Core structure to register new commands
    * or to dispatch existing ones
    */
struct command_registry_t {
    command_node_t root;

    // switches discovered during dispatch
    std::vector<std::string> switches;

    // simple utility to query active switches
    bool is_set(const std::string& option) const {
        for (const auto& s : switches) {
            if (s == option) return true;
        }
        return false;
    }
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
};



/**
 * @brief Attaches new command to existing registry
 * @param reg - Existing command registry
 * @param cmd - Template for new command entry
 * @return status_t
 * VM_OK on success
 */
::thaumaturgy::yield execute(proc_exec_unit_t& peu);


/**
 * @brief Attaches new command to existing registry
 * @param reg - Existing command registry
 * @param cmd - Template for new command entry
 * @return status_t
 * VM_OK on success
 */
::thaumaturgy::yield register_command(command_registry_t& reg, const command_t& cmd);

/**
 * @brief Attaches many commands at once
 * @param reg - Existing command registry
 * @param cmds - Vector of commands
 * @return status_t
 * VM_OK on success
 */
::thaumaturgy::yield register_command(command_registry_t& reg, const std::vector<command_t>& cmds);

/**
 * @brief Dispatch a command formed from shell invokation arguments
 * @param reg - Existing registry
 * @param argc - taken from main arguments
 * @param argv - taken from main arguments
 * @return status_t
 */
::thaumaturgy::yield dispatch_command(const command_registry_t& reg, const int argc, const char** argv);

/**
 * @brief Dispatch a command in a string form
 * @param reg - Existing registry
 * @param input - Command in a string form
 * @return status_t
 */
::thaumaturgy::yield dispatch_command(const command_registry_t& reg, std::string input);

 
 /**
  * @brief Walk tree and execute appropriate handler using leaf-first logic.
  * @param reg - Existing registry
  * @param input - Command in a string form
  * @return status_t
  */
::thaumaturgy::yield execute_command(const command_registry_t& runtime_registry,const std::vector<std::string>& tokens);

/**
 * @brief Split a string into tokens
 * @param input - single string to be split
 * @return std::vector<std::string> of separate words
 */
std::vector<std::string> tokenize(std::string input);

// Remove switches from token stream, push to registry.switches.
void extract_switches(std::vector<std::string>& tokens, command_registry_t& out_runtime);

} // namespace sigil::platform
