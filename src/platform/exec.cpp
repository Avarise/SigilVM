#include <sigil/platform/exec.h>
#include <sigil/common.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sstream>
#include <fcntl.h>

extern "C" {
    extern char **environ;
}

// Local helper, TODO: move to the public headers
bool starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}


namespace sigil::platform {

static int open_devnull(int flags) {
    return open("/dev/null", flags);
}

static int create_memfd(const char* name) {
#   if defined(__linux__)
    return memfd_create(name, MFD_CLOEXEC);
#   else
    (void)name;
    errno = ENOSYS;
    return -1;
#   endif
}

static void normalize_wait_status(int status, proc_exec_result_t& out) {
    if (WIFEXITED(status)) {
        out.exit_code  = WEXITSTATUS(status);
        out.term_signal = 0;
    } else if (WIFSIGNALED(status)) {
        out.exit_code  = -1;
        out.term_signal = WTERMSIG(status);
    } else {
        out.exit_code  = -1;
        out.term_signal = 0;
    }
}


::sigil::yield execute(proc_exec_unit_t& peu) {
    if (peu.target.empty()) {
        return ::sigil::yield().set_state(sigil::yield_state::fail);
    }

    std::vector<char*> argv;
    argv.reserve(peu.args.size() + 2);

    argv.push_back(const_cast<char*>(peu.target.c_str()));
    for (auto& a : peu.args) {
        argv.push_back(const_cast<char*>(a.c_str()));
    }
    argv.push_back(nullptr);

    std::vector<char*> envp;
    if (!peu.exports.empty()) {
        for (char** e = environ; *e; ++e) {
            envp.push_back(*e);
        }
        for (auto& e : peu.exports) {
            envp.push_back(const_cast<char*>(e.c_str()));
        }
        envp.push_back(nullptr);
    }

    int stdout_fd = -1;
    int stderr_fd = -1;

    const bool want_capture =
        (peu.exec_mode == EXEC_WAIT) &&
        (peu.stdout_mode == STDIO_CAPTURE ||
         peu.stderr_mode == STDIO_CAPTURE);

    if (want_capture) {
        if (peu.stdout_mode == STDIO_CAPTURE) {
            stdout_fd = create_memfd("sigil-exec-stdout");
            if (stdout_fd < 0)
                return ::sigil::yield().set_state(sigil::yield_state::fail);
        }
        if (peu.stderr_mode == STDIO_CAPTURE) {
            stderr_fd = create_memfd("sigil-exec-stderr");
            if (stderr_fd < 0)
                return ::sigil::yield().set_state(sigil::yield_state::fail);
        }
    }

    pid_t pid = fork();
    if (pid < 0) {
        return ::sigil::yield().set_state(sigil::yield_state::fail);
    }

    if (pid == 0) {
        if (peu.exec_mode == EXEC_DETACH) {
            setsid();
        }

        if (!peu.workdir.empty()) {
            std::cout << "Changing Work dir to: " << peu.workdir.c_str() << std::endl;
            if (chdir(peu.workdir.c_str()) != 0) {
                _exit(127);
            }
        }

        if (peu.stdin_mode == STDIO_NULL) {
            int dn = open_devnull(O_RDONLY);
            if (dn >= 0) {
                dup2(dn, STDIN_FILENO);
                close(dn);
            }
        }

        if (peu.stdout_mode == STDIO_NULL) {
            int dn = open_devnull(O_WRONLY);
            if (dn >= 0) {
                dup2(dn, STDOUT_FILENO);
                close(dn);
            }
        } else if (peu.stdout_mode == STDIO_CAPTURE) {
            dup2(stdout_fd, STDOUT_FILENO);
        }

        if (peu.stderr_mode == STDIO_NULL) {
            int dn = open_devnull(O_WRONLY);
            if (dn >= 0) {
                dup2(dn, STDERR_FILENO);
                close(dn);
            }
        } else if (peu.stderr_mode == STDIO_CAPTURE) {
            dup2(stderr_fd, STDERR_FILENO);
        }

        if (!peu.exports.empty()) {
            if (peu.use_path) {
                execvpe(peu.target.c_str(), argv.data(), envp.data());
            } else {
                execve(peu.target.c_str(), argv.data(), envp.data());
            }
        } else {
            if (peu.use_path) {
                execvp(peu.target.c_str(), argv.data());
            } else {
                execv(peu.target.c_str(), argv.data());
            }
        }

        _exit(127);
    }

    peu.result.pid = pid;

    if (peu.exec_mode == EXEC_REPLACE) {
        return ::sigil::yield();
    }

    if (peu.exec_mode == EXEC_DETACH) {
        return ::sigil::yield();
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        return ::sigil::yield().set_state(sigil::yield_state::fail);
    }

    normalize_wait_status(status, peu.result);

    if (stdout_fd >= 0) {
        lseek(stdout_fd, 0, SEEK_SET);
        peu.result.stdout_fd = stdout_fd;
    }

    if (stderr_fd >= 0) {
        lseek(stderr_fd, 0, SEEK_SET);
        peu.result.stderr_fd = stderr_fd;
    }

    return ::sigil::yield();
}


::sigil::yield
register_command(command_registry_t& reg, const command_t& cmd) {

    command_node_t* node = &reg.root;

    // Descend only if words are provided
    if (!cmd.words.empty()) {
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
    }

    // Prevent handler clobbering (including root)
    if (node->handler != nullptr) {
        return ::sigil::yield().set_state(sigil::yield_state::fail);
    }

    node->handler = cmd.handler;
    node->passthrough = cmd.passthrough ? true : false;

    return ::sigil::yield();
}


::sigil::yield
register_command(command_registry_t& reg, const std::vector<command_t>& cmds) {
    ::sigil::yield accum;
    for (const auto& c : cmds) {
        ::sigil::yield s = register_command(reg, c);
        if (s.is_failure()) {
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

void extract_switches(std::vector<std::string>& tokens, command_registry_t& out_runtime) {
    std::vector<std::string> filtered;
    filtered.reserve(tokens.size());

    for (auto& tok : tokens) {
        if (starts_with(tok, "--")) {
            switch_arg_t sw;

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


::sigil::yield execute_command
(const command_registry_t& runtime_registry, const std::vector<std::string>& tokens) {
    ::sigil::yield ret;

    command_node_t* node =
        const_cast<command_node_t*>(&runtime_registry.root);

    std::size_t idx = 0;
    const std::size_t n = tokens.size();

    // Special case: no positional tokens
    if (tokens.empty()) {
        if (node->handler != nullptr) {
            ::sigil::platform::cmd_handler_args_t args;
            args.switches = runtime_registry.switches;
            return node->handler(args);
        }
        return ret.set_state(sigil::yield_state::fail);
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

        ::sigil::platform::cmd_handler_args_t handler_args;
        handler_args.switches = runtime_registry.switches;

        if (leftover_count == 0) {
            return node->handler(handler_args);
        }

        if (node->passthrough) {
            handler_args.args.reserve(leftover_count);
            for (std::size_t i = idx; i < n; ++i)
                handler_args.args.push_back(tokens[i]);

            return node->handler(handler_args);
        }

        return ret.set_state(sigil::yield_state::fail);
    }

    // No handler at this node
    if (!node->children.empty()) {
        return ret.set_state(sigil::yield_state::fail);
    }

    return ret.set_state(sigil::yield_state::fail);
}


::sigil::yield dispatch_command
(const command_registry_t& reg, const int argc, const char** argv) {
    ::sigil::yield st;
    // if (argc <= 1) {
    //     return st.set_state(sigil::yield_state::fail);
    // }

    // build tokens skipping argv[0] (program name)
    std::vector<std::string> tokens;
    tokens.reserve(static_cast<std::size_t>(argc - 1));
    for (int i = 1; i < argc; ++i) {
        if (argv[i] != nullptr) tokens.emplace_back(argv[i]);
    }

    // make runtime registry copy (shares tree pointers)
    command_registry_t runtime = reg;

    // extract switches into runtime.switches, tokens filtered
    extract_switches(tokens, runtime);

    // execute
    return execute_command(runtime, tokens);
}

::sigil::yield
dispatch_command(const command_registry_t& reg, std::string input) {
    auto tokens = tokenize(input);

    // make runtime registry copy
    command_registry_t runtime = reg;

    extract_switches(tokens, runtime);

    return execute_command(runtime, tokens);
}



} // namespace sigil::platform
