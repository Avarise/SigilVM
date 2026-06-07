#pragma once

/**
 * SigilVM Header for execution of system binaries.
 * Centered around Process Execution Unit, which provides
 * options and structures to choose what to do with process
 * I/O files, storage of return code, and exports + argument passing.
 */

#include <sys/stat.h>
#include <unordered_map>
#include <sys/types.h>
#include <unistd.h>
#include <optional>
#include <cstdint>
#include <string>
#include <vector>

#include "../core/status.h"


namespace sigilvm::runtime {

struct switch_arg_t {
    std::string name;                    // e.g., "--verbose"
    std::optional<std::string> value;    // std::nullopt if boolean switch
};
    
struct cmd_handler_args_t {
    std::vector<std::string> args;
    std::vector<switch_arg_t> switches;
};
    
using command_handler_t = ::sigilvm::status_t (*)(const cmd_handler_args_t);


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
    std::vector<switch_arg_t> switches;

    // simple utility to query active switches
    bool is_set(const std::string& option) const {
        for (const auto& s : switches) {
            if (s.name == option) return true;
        }
        return false;
    }

    status_t register_command(const command_t &cmd);
    status_t register_command(const std::vector<command_t> &cmds);
    status_t dispatch_command(const int argc, const char *argv[]);
    status_t dispatch_command(const std::string cmd);
    status_t execute_command(const std::vector<std::string> &tokens);
    status_t extract_switches(std::vector<std::string> &tokens);
};    
    

/**
 * @brief Split a string into tokens
 * @param input - single string to be split
 * @return std::vector<std::string> of separate words
 */
std::vector<std::string> tokenize(std::string input);

} // namespace sigil::platform
