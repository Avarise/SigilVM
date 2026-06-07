#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sstream>
#include <fcntl.h>
#include "../../include/runtime/commands.h"
#include "../../include/core/strings.h"

extern "C" {
    extern char **environ;
}

namespace sigilvm::runtime {

::sigilvm::status_t 
command_registry_t::register_command(const ::sigilvm::runtime:: command_t& cmd) {
    ::sigilvm::status_t st;
    auto *node = &this->root;

    // Descend only if words are provided
    if (!cmd.words.empty()) {
        for (const auto& w : cmd.words) {
            auto it = node->children.find(w);
            if (it == node->children.end()) {
                ::sigilvm::runtime::command_node_t* child = new ::sigilvm::runtime::command_node_t;
                node->children.emplace(w, child);
                node = child;
            } else {
                node = it->second;
            }
        }
    }

    // Prevent handler clobbering (including root)
    if (node->handler != nullptr) {
        return st.set_state(::sigilvm::state::fail);
    }

    node->handler = cmd.handler;
    node->passthrough = cmd.passthrough ? true : false;

    return st;
}


::sigilvm::status_t
command_registry_t::register_command(const std::vector<::sigilvm::runtime::command_t>&cmds) {
    ::sigilvm::status_t st;
    for (const auto &c : cmds) {
        st = this->register_command(c);
        if (st.is_failure()) {
            return st;
        }
    }
    return st;
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

void extract_switches(std::vector<std::string>& tokens, ::sigilvm::runtime::command_registry_t& out_runtime) {
    std::vector<std::string> filtered;
    filtered.reserve(tokens.size());

    for (auto& tok : tokens) {
        if (::sigilvm::strings::starts_with(tok, "--")) {
            ::sigilvm::runtime::switch_arg_t sw;

            auto eq_pos = tok.find('=');
            if (eq_pos != std::string::npos) {
                sw.name = tok.substr(0, eq_pos);
                std::string val = tok.substr(eq_pos + 1);

                if (val.empty()) {
                    throw std::runtime_error("Switch '" + sw.name + "' requires a value after '='");
                }

                sw.value = val;
            } else {
                sw.name = tok;
                sw.value = std::nullopt;
            }

            out_runtime.switches.push_back(std::move(sw));
        } else {
            filtered.push_back(tok);
        }
    }

    tokens.swap(filtered);
}


::sigilvm::status_t 
command_registry_t::execute_command
(const std::vector<std::string>& tokens) {
    ::sigilvm::status_t st;

    auto *node = const_cast<::sigilvm::runtime::command_node_t*>(&this->root);

    std::size_t idx = 0;
    const std::size_t n = tokens.size();

    // Special case: no positional tokens
    if (tokens.empty()) {
        if (node->handler != nullptr) {
            ::sigilvm::runtime::cmd_handler_args_t args;
            args.switches = this->switches;
            return node->handler(args);
        }
        return st.set_state(sigilvm::state::fail);
    }

    while (idx < n) {
        auto it = node->children.find(tokens[idx]);
        if (it == node->children.end())
            break;
        node = it->second;
        ++idx;
    }

    const std::size_t leftover_count = n - idx;

    if (node->handler != nullptr) {

        ::sigilvm::runtime::cmd_handler_args_t handler_args;
        handler_args.switches = this->switches;

        if (leftover_count == 0) {
            return node->handler(handler_args);
        }

        if (node->passthrough) {
            handler_args.args.reserve(leftover_count);
            for (std::size_t i = idx; i < n; ++i)
                handler_args.args.push_back(tokens[i]);

            return node->handler(handler_args);
        }

        return st.set_state(sigilvm::state::fail);
    }

    // No handler at this node
    if (!node->children.empty()) {
        return st.set_state(sigilvm::state::fail);
    }

    return st.set_state(sigilvm::state::fail);
}


::sigilvm::status_t 
command_registry_t::dispatch_command
(const int argc, const char** argv) {
    ::sigilvm::status_t st;

    // build tokens skipping argv[0] (program name)
    std::vector<std::string> tokens;
    tokens.reserve(static_cast<std::size_t>(argc - 1));
    for (int i = 1; i < argc; ++i) {
        if (argv[i] != nullptr) tokens.emplace_back(argv[i]);
    }

    // make runtime registry copy (shares tree pointers)
    auto runtime = *this;
    runtime.extract_switches(tokens);
    return runtime.execute_command(tokens);
}

::sigilvm::status_t 
command_registry_t::dispatch_command(std::string input) {
    auto tokens = tokenize(input);
    auto runtime = *this;
    runtime.extract_switches(tokens);
    return runtime.execute_command(tokens);
}

} // namespace sigil::runtime
