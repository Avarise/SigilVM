#include <sigil/platform/exec.h>
#include <sigil/status.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <fcntl.h>
#include <cerrno>

extern "C" {
    extern char **environ;
}


namespace sigil::platform {


// ------------------------------------------------------------
// helpers
// ------------------------------------------------------------

static int open_devnull(int flags) {
    return open("/dev/null", flags);
}

static int create_memfd(const char* name) {
#if defined(__linux__)
    return memfd_create(name, MFD_CLOEXEC);
#else
    (void)name;
    errno = ENOSYS;
    return -1;
#endif
}

static void normalize_wait_status(int status,
                                  proc_exec_result_t& out) {
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


// ------------------------------------------------------------
// execute
// ------------------------------------------------------------

sigil::status_t execute(proc_exec_unit_t& peu) {
    if (peu.target.empty()) {
        return sigil::VM_ARG_NULL;
    }

    //
    // build argv
    //
    std::vector<char*> argv;
    argv.reserve(peu.args.size() + 2);

    argv.push_back(const_cast<char*>(peu.target.c_str()));
    for (auto& a : peu.args) {
        argv.push_back(const_cast<char*>(a.c_str()));
    }
    argv.push_back(nullptr);

    //
    // build envp
    //
    std::vector<char*> envp;
    if (!peu.exports.empty()) {
        //extern char **environ;
        for (char** e = environ; *e; ++e) {
            envp.push_back(*e);
        }
        for (auto& e : peu.exports) {
            envp.push_back(const_cast<char*>(e.c_str()));
        }
        envp.push_back(nullptr);
    }

    //
    // prepare capture FDs (WAIT only)
    //
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
                return sigil::VM_FAILED;
        }
        if (peu.stderr_mode == STDIO_CAPTURE) {
            stderr_fd = create_memfd("sigil-exec-stderr");
            if (stderr_fd < 0)
                return sigil::VM_FAILED;
        }
    }

    //
    // fork
    //
    pid_t pid = fork();
    if (pid < 0) {
        return sigil::VM_FAILED;
    }

    //
    // child
    //
    if (pid == 0) {
        //
        // detach if requested
        //
        if (peu.exec_mode == EXEC_DETACH) {
            setsid();
        }

        //
        // working directory
        //
        if (!peu.workdir.empty()) {
            if (chdir(peu.workdir.c_str()) != 0) {
                _exit(127);
            }
        }

        //
        // stdin
        //
        if (peu.stdin_mode == STDIO_NULL) {
            int dn = open_devnull(O_RDONLY);
            if (dn >= 0) {
                dup2(dn, STDIN_FILENO);
                close(dn);
            }
        }

        //
        // stdout
        //
        if (peu.stdout_mode == STDIO_NULL) {
            int dn = open_devnull(O_WRONLY);
            if (dn >= 0) {
                dup2(dn, STDOUT_FILENO);
                close(dn);
            }
        } else if (peu.stdout_mode == STDIO_CAPTURE) {
            dup2(stdout_fd, STDOUT_FILENO);
        }

        //
        // stderr
        //
        if (peu.stderr_mode == STDIO_NULL) {
            int dn = open_devnull(O_WRONLY);
            if (dn >= 0) {
                dup2(dn, STDERR_FILENO);
                close(dn);
            }
        } else if (peu.stderr_mode == STDIO_CAPTURE) {
            dup2(stderr_fd, STDERR_FILENO);
        }

        //
        // exec
        //
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

    //
    // parent
    //
    peu.result.pid = pid;

    //
    // EXEC_REPLACE never returns on success
    //
    if (peu.exec_mode == EXEC_REPLACE) {
        return sigil::VM_OK;
    }

    //
    // EXEC_DETACH does not wait
    //
    if (peu.exec_mode == EXEC_DETACH) {
        return sigil::VM_OK;
    }

    //
    // EXEC_WAIT
    //
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        return sigil::VM_FAILED;
    }

    normalize_wait_status(status, peu.result);

    //
    // rewind capture fds for reading
    //
    if (stdout_fd >= 0) {
        lseek(stdout_fd, 0, SEEK_SET);
        peu.result.stdout_fd = stdout_fd;
    }

    if (stderr_fd >= 0) {
        lseek(stderr_fd, 0, SEEK_SET);
        peu.result.stderr_fd = stderr_fd;
    }

    return sigil::VM_OK;
}

} // namespace sigil::platform
