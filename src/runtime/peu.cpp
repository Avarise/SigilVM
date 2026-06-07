#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "../../include/core/status.h"
#include "../../include/runtime/peu.h"


extern "C" {
    extern char **environ;
}

namespace sigilvm::runtime {
    
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

status_t proc_exec_unit_t::run() {
    status_t st;
    
    if (this->target.empty()) {
        return st.set_state(::sigilvm::state::fail);
    }

    std::vector<char*> argv;
    argv.reserve(this->args.size() + 2);

    argv.push_back(const_cast<char*>(this->target.c_str()));
    for (auto& a : this->args) {
        argv.push_back(const_cast<char*>(a.c_str()));
    }
    argv.push_back(nullptr);

    std::vector<char*> envp;
    if (!this->exports.empty()) {
        for (char** e = environ; *e; ++e) {
            envp.push_back(*e);
        }
        for (auto& e : this->exports) {
            envp.push_back(const_cast<char*>(e.c_str()));
        }
        envp.push_back(nullptr);
    }

    int stdout_fd = -1;
    int stderr_fd = -1;

    const bool want_capture =
        (this->exec_mode == EXEC_WAIT) &&
        (this->stdout_mode == STDIO_CAPTURE ||
         this->stderr_mode == STDIO_CAPTURE);

    if (want_capture) {
        if (this->stdout_mode == STDIO_CAPTURE) {
            stdout_fd = create_memfd("sigil-exec-stdout");
            if (stdout_fd < 0)
                return st.set_state(::sigilvm::state::fail);
        }
        if (this->stderr_mode == STDIO_CAPTURE) {
            stderr_fd = create_memfd("sigil-exec-stderr");
            if (stderr_fd < 0)
                return st.set_state(::sigilvm::state::fail);
        }
    }

    pid_t pid = fork();
    if (pid < 0) {
        return st.set_state(::sigilvm::state::fail);
    }

    if (pid == 0) {
        if (this->exec_mode == EXEC_DETACH) {
            setsid();
        }

        if (!this->workdir.empty()) {
            if (chdir(this->workdir.c_str()) != 0) {
                _exit(127);
            }
        }

        if (this->stdin_mode == STDIO_NULL) {
            int dn = open_devnull(O_RDONLY);
            if (dn >= 0) {
                dup2(dn, STDIN_FILENO);
                close(dn);
            }
        }

        if (this->stdout_mode == STDIO_NULL) {
            int dn = open_devnull(O_WRONLY);
            if (dn >= 0) {
                dup2(dn, STDOUT_FILENO);
                close(dn);
            }
        } else if (this->stdout_mode == STDIO_CAPTURE) {
            dup2(stdout_fd, STDOUT_FILENO);
        }

        if (this->stderr_mode == STDIO_NULL) {
            int dn = open_devnull(O_WRONLY);
            if (dn >= 0) {
                dup2(dn, STDERR_FILENO);
                close(dn);
            }
        } else if (this->stderr_mode == STDIO_CAPTURE) {
            dup2(stderr_fd, STDERR_FILENO);
        }

        if (!this->exports.empty()) {
            if (this->use_path) {
                execvpe(this->target.c_str(), argv.data(), envp.data());
            } else {
                execve(this->target.c_str(), argv.data(), envp.data());
            }
        } else {
            if (this->use_path) {
                execvp(this->target.c_str(), argv.data());
            } else {
                execv(this->target.c_str(), argv.data());
            }
        }

        _exit(127);
    }

    this->result.pid = pid;

    if (this->exec_mode == EXEC_REPLACE) {
        return st;
    }

    if (this->exec_mode == EXEC_DETACH) {
        return st;
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        return st.set_state(::sigilvm::state::fail);
    }

    normalize_wait_status(status, this->result);

    if (stdout_fd >= 0) {
        lseek(stdout_fd, 0, SEEK_SET);
        this->result.stdout_fd = stdout_fd;
    }

    if (stderr_fd >= 0) {
        lseek(stderr_fd, 0, SEEK_SET);
        this->result.stderr_fd = stderr_fd;
    }

    return st;
}
   
}
