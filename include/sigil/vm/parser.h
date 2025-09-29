#pragma once

#include <sigil/status.h>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>


namespace sigil::vm {

// -----------------------------------------------------------------------------
// Handler function signature
// -----------------------------------------------------------------------------
using command_handler_t = status_t (*)(const std::vector<std::string>& args);

// -----------------------------------------------------------------------------
// Command registration input descriptor
// -----------------------------------------------------------------------------
struct command_t {
    std::vector<std::string> words;     // command tokens: {"theme","set"}
    bool passthrough;                   // allow remaining args to be passed
    command_handler_t handler;          // nullptr = invalid

    command_t(const std::initializer_list<std::string> &w,
              bool pass,
              command_handler_t h) noexcept
        : words(w), passthrough(pass), handler(h) {}
};

// -----------------------------------------------------------------------------
// Internal command tree node
// -----------------------------------------------------------------------------
struct command_node_t {
    std::unordered_map<std::string, command_node_t*> children;
    command_handler_t handler;
    bool passthrough;

    command_node_t()
        : handler(nullptr), passthrough(false)
    {}
};

// -----------------------------------------------------------------------------
// Registry storing the full command tree and switch state
// -----------------------------------------------------------------------------
struct command_registry_t {
    command_node_t root;

    // switches discovered during dispatch
    std::vector<std::string> switches;

    // known global switches, e.g. {"--debug","--force"}
    //std::unordered_set<std::string> valid_switches;

    // simple utility to query active switches
    bool is_set(const std::string& option) const {
        for (const auto& s : switches) {
            if (s == option) return true;
        }
        return false;
    }
};

// -----------------------------------------------------------------------------
// Registration API
// -----------------------------------------------------------------------------
status_t register_command(command_registry_t& reg,
                          const command_t& cmd);

status_t register_command(command_registry_t& reg,
                          const std::vector<command_t>& cmds);

// -----------------------------------------------------------------------------
// Dispatch API (argv or text)
// -----------------------------------------------------------------------------
status_t dispatch_command(const command_registry_t& reg,
                          const int argc, const char** argv);

status_t dispatch_command(const command_registry_t& reg,
                          std::string input);

// -----------------------------------------------------------------------------
// Optional helpers (implemented in .cpp)
// -----------------------------------------------------------------------------

// Split input into tokens using whitespace rules.
std::vector<std::string> tokenize(std::string input);

// Remove switches from token stream, push to registry.switches.
void extract_switches(const command_registry_t& reg,
                      std::vector<std::string>& tokens,
                      command_registry_t& out_runtime);

// Walk tree and execute appropriate handler using leaf-first logic.
status_t execute_command(const command_registry_t& runtime_registry,
                         const std::vector<std::string>& tokens);

} // namespace sigil
