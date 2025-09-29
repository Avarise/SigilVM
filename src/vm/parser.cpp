#include <sigil/vm/parser.h>
#include <sigil/status.h>
#include <sstream>

namespace sigil::vm {

status_t register_command(command_registry_t& reg, const command_t& cmd) {
    if (cmd.words.empty()) return VM_ARG_INVALID;
    
    command_node_t* node = &reg.root;

    for (const auto& w : cmd.words) {
        auto it = node->children.find(w);
        if (it == node->children.end()) {
            command_node_t* child = new command_node_t;
            node->children.emplace(w, child);
            node = child;
        } else {
            node = it->second;
        }
    }

    // at final node: ensure not clobbering existing handler
    if (node->handler != nullptr) {
        return VM_ALREADY_EXISTS;
    }

    node->handler = cmd.handler;
    node->passthrough = cmd.passthrough ? true : false;

    return VM_OK;
}

status_t register_command(command_registry_t& reg, const std::vector<command_t>& cmds) {
    status_t accum = VM_OK;
    for (const auto& c : cmds) {
        status_t s = register_command(reg, c);
        if (s != VM_OK) {
            return s;
        } else {
            accum = s;
        }
    }
    return accum;
}

std::vector<std::string> tokenize(std::string input) {
    std::vector<std::string> out;
    std::istringstream ss(input);
    std::string tok;
    while (ss >> tok) {
        out.push_back(std::move(tok));
    }
    return out;
}

void extract_switches(const command_registry_t& reg,
                      std::vector<std::string>& tokens,
                      command_registry_t& out_runtime) {
    std::vector<std::string> filtered;
    filtered.reserve(tokens.size());

    for (const auto& tok : tokens) {
        if (tok.size() >= 3 && tok[0] == '-' && tok[1] == '-') {
            // Only treat as switch if in valid set
            out_runtime.switches.push_back(tok);
            continue; // drop from token stream
            // if (reg.valid_switches.find(tok) != reg.valid_switches.end()) {
            // }
        }
        filtered.push_back(tok);
    }

    tokens.swap(filtered);
}

// -----------------------------------------------------------------------------
// execute_command
//   Walk as deep as possible. After stopping:
//     - if node->handler present:
//         * if no leftover tokens -> call handler({})
//         * else if node->passthrough -> call handler(leftover)
//         * else -> return VM_ARG_INVALID
//     - else (no handler):
//         * if node has children -> missing argument (VM_ARG_INVALID)
//         * else -> unknown command (VM_FORMAT_INVALID)
// -----------------------------------------------------------------------------
status_t execute_command(const command_registry_t& runtime_registry,
                         const std::vector<std::string>& tokens) {
    // quick path: no tokens
    if (tokens.empty()) {
        return VM_FORMAT_INVALID;
    }

    command_node_t* node = const_cast<command_node_t*>(&runtime_registry.root);
    std::size_t idx = 0;
    std::size_t n = tokens.size();

    // descend while child exists for current token
    while (idx < n) {
        auto it = node->children.find(tokens[idx]);
        if (it == node->children.end()) break;
        node = it->second;
        ++idx;
    }

    // idx tokens were consumed as a path; leftover = tokens[idx..end)
    const std::size_t leftover_count = n - idx;

    if (node->handler != nullptr) {
        // handler exists on matched node
        if (leftover_count == 0) {
            // call with empty args
            std::vector<std::string> empty_args;
            return node->handler(empty_args);
        } else {
            // leftover exists
            if (node->passthrough) {
                std::vector<std::string> leftover;
                leftover.reserve(leftover_count);
                for (std::size_t i = idx; i < n; ++i) leftover.push_back(tokens[i]);
                return node->handler(leftover);
            } else {
                return VM_ARG_INVALID;
            }
        }
    } else {
        // no handler at this node
        if (!node->children.empty()) {
            // There are deeper possible subcommands -> user didn't provide enough tokens
            return VM_ARG_INVALID;
        } else {
            // No handler and no children: the matched path leads to nothing useful (should be unknown)
            // If idx == 0, the very first token didn't match any root child -> unknown command
            return VM_FORMAT_INVALID;
        }
    }
}

// -----------------------------------------------------------------------------
// dispatch_command (argc/argv)
// -----------------------------------------------------------------------------
status_t dispatch_command(const command_registry_t& reg,
                          const int argc, const char** argv) {
    if (argc <= 1) {
        return VM_FORMAT_INVALID;
    }

    // build tokens skipping argv[0] (program name)
    std::vector<std::string> tokens;
    tokens.reserve(static_cast<std::size_t>(argc - 1));
    for (int i = 1; i < argc; ++i) {
        if (argv[i] != nullptr) tokens.emplace_back(argv[i]);
    }

    // make runtime registry copy (shares tree pointers)
    command_registry_t runtime = reg;

    // extract switches into runtime.switches, tokens filtered
    extract_switches(reg, tokens, runtime);

    // execute
    return execute_command(runtime, tokens);
}

// -----------------------------------------------------------------------------
// dispatch_command (string input)
// -----------------------------------------------------------------------------
status_t dispatch_command(const command_registry_t& reg,
                          std::string input) {
    auto tokens = tokenize(input);

    // make runtime registry copy
    command_registry_t runtime = reg;

    extract_switches(reg, tokens, runtime);

    return execute_command(runtime, tokens);
}

} // namespace sigil
