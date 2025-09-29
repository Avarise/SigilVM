#pragma once

#include <sigil/status.h>
#include <unordered_map>
#include <string>
#include <vector>


namespace sigil::vm {

using command_handler_t = status_t (*)(const std::vector<std::string>& args);

/**
 * @brief
 * User-facing structure used to register new command
 */
struct command_t {
    std::vector<std::string> words;     // command tokens: {"theme","set"}
    bool passthrough;                   // allow remaining args to be passed
    command_handler_t handler;          // nullptr = invalid

    command_t(const std::initializer_list<std::string> &w,
              bool pass,
              command_handler_t h) noexcept
        : words(w), passthrough(pass), handler(h) {}
};

/**
 * @brief
 * Structure for creation of internal command tree
 */
struct command_node_t {
    std::unordered_map<std::string, command_node_t*> children;
    command_handler_t handler;
    bool passthrough;

    command_node_t()
        : handler(nullptr), passthrough(false)
    {}
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

/**
 * @brief Attaches new command to existing registry
 * @param reg - Existing command registry
 * @param cmd - Template for new command entry
 * @return status_t
 * VM_OK on success
 */
status_t register_command(command_registry_t& reg, const command_t& cmd);

/**
 * @brief Attaches many commands at once
 * @param reg - Existing command registry
 * @param cmds - Vector of commands
 * @return status_t
 * VM_OK on success
 */
status_t register_command(command_registry_t& reg,
                          const std::vector<command_t>& cmds);

/**
 * @brief Dispatch a command formed from shell invokation arguments
 * @param reg - Existing registry
 * @param argc - taken from main arguments
 * @param argv - taken from main arguments
 * @return status_t
 */
status_t dispatch_command(const command_registry_t& reg, const int argc, const char** argv);

/**
 * @brief Dispatch a command in a string form
 * @param reg - Existing registry
 * @param input - Command in a string form
 * @return status_t
 */
status_t dispatch_command(const command_registry_t& reg, std::string input);

 
 /**
  * @brief Walk tree and execute appropriate handler using leaf-first logic.
  * @param reg - Existing registry
  * @param input - Command in a string form
  * @return status_t
  */
status_t execute_command(const command_registry_t& runtime_registry,const std::vector<std::string>& tokens);

/**
 * @brief Split a string into tokens
 * @param input - single string to be split
 * @return std::vector<std::string> of separate words
 */
std::vector<std::string> tokenize(std::string input);

// Remove switches from token stream, push to registry.switches.
void extract_switches(const command_registry_t& reg,
                      std::vector<std::string>& tokens,
                      command_registry_t& out_runtime);



} // namespace sigil
