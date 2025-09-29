#include <sigil/platform/exec.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>

using namespace sigil::platform;

static std::string read_all_fd(int fd) {
    std::string out;
    if (fd < 0)
        return out;

    char buf[4096];
    ssize_t n;

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        out.append(buf, n);
    }

    return out;
}

static std::filesystem::path make_temp_dir() {
    std::filesystem::path base = std::filesystem::temp_directory_path();
    std::filesystem::path dir;

    for (int i = 0; i < 100; ++i) {
        dir = base / ("sigil-peu-test-" + std::to_string(getpid()) + "-" + std::to_string(i));
        if (!std::filesystem::exists(dir)) {
            std::filesystem::create_directory(dir);
            return dir;
        }
    }

    return {};
}

TEST(PEU, ExecWaitTrue) {
    proc_exec_unit_t peu;
    peu.set_target("/bin/true");
    peu.exec_mode = EXEC_WAIT;

    ::thaumaturgy::yield s = execute(peu);
    ASSERT_EQ(s.is_ok(), true);
    ASSERT_EQ(peu.result.exit_code, 0);
    ASSERT_EQ(peu.result.term_signal, 0);
}

TEST(PEU, ExecWaitFalse) {
    proc_exec_unit_t peu;
    peu.set_target("/bin/false");
    peu.exec_mode = EXEC_WAIT;

    ::thaumaturgy::yield s = execute(peu);
    ASSERT_EQ(s.is_ok(), true);
    ASSERT_EQ(peu.result.exit_code, 1);
}

TEST(PEU, ExecEchoArgumentCapture) {
    proc_exec_unit_t peu;
    peu.set_target("/bin/echo")
       .push_argument("hello");

    peu.exec_mode   = EXEC_WAIT;
    peu.stdout_mode = STDIO_CAPTURE;

    ::thaumaturgy::yield s = execute(peu);
    ASSERT_EQ(s.is_ok(), true);
    ASSERT_GE(peu.result.stdout_fd, 0);

    std::string out = read_all_fd(peu.result.stdout_fd);
    ASSERT_EQ(out, "hello\n");
}

TEST(PEU, ExecStdoutStderrSeparation) {
    proc_exec_unit_t peu;
    peu.set_target("/bin/sh")
       .push_argument("-c")
       .push_argument("echo out; echo err 1>&2");

    peu.exec_mode   = EXEC_WAIT;
    peu.stdout_mode = STDIO_CAPTURE;
    peu.stderr_mode = STDIO_CAPTURE;

    ::thaumaturgy::yield s = execute(peu);
    ASSERT_EQ(s.is_ok(), true);

    std::string out = read_all_fd(peu.result.stdout_fd);
    std::string err = read_all_fd(peu.result.stderr_fd);

    ASSERT_EQ(out, "out\n");
    ASSERT_EQ(err, "err\n");
}

TEST(PEU, ExecStdoutNull) {
    proc_exec_unit_t peu;
    peu.set_target("/bin/echo")
       .push_argument("discard");

    peu.exec_mode   = EXEC_WAIT;
    peu.stdout_mode = STDIO_NULL;

    ::thaumaturgy::yield s = execute(peu);
    ASSERT_EQ(s.is_ok(), true);
    ASSERT_EQ(peu.result.exit_code, 0);
    ASSERT_LT(peu.result.stdout_fd, 0);
}

TEST(PEU, ExecEnvExport) {
    proc_exec_unit_t peu;
    peu.set_target("/bin/sh")
       .push_argument("-c")
       .push_argument("echo $SIGIL_TEST_VAR")
       .export_var("SIGIL_TEST_VAR", "ok");

    peu.exec_mode   = EXEC_WAIT;
    peu.stdout_mode = STDIO_CAPTURE;

    ::thaumaturgy::yield s = execute(peu);
    ASSERT_EQ(s.is_ok(), true);

    std::string out = read_all_fd(peu.result.stdout_fd);
    ASSERT_EQ(out, "ok\n");
}

TEST(PEU, ExecWorkingDirectory) {
    std::filesystem::path dir = make_temp_dir();
    ASSERT_FALSE(dir.empty());

    proc_exec_unit_t peu;
    peu.set_target("/bin/pwd");
    peu.exec_mode   = EXEC_WAIT;
    peu.stdout_mode = STDIO_CAPTURE;
    peu.workdir     = dir.string();

    ::thaumaturgy::yield s = execute(peu);
    ASSERT_EQ(s.is_ok(), true);

    std::string out = read_all_fd(peu.result.stdout_fd);

    // pwd prints newline
    if (!out.empty() && out.back() == '\n')
        out.pop_back();

    ASSERT_EQ(out, dir.string());
}

TEST(PEU, ExecDetachDoesNotWait) {
    proc_exec_unit_t peu;
    peu.set_target("/bin/sleep")
       .push_argument("1");

    peu.exec_mode = EXEC_DETACH;

    ::thaumaturgy::yield s = execute(peu);
    ASSERT_EQ(s.is_ok(), true);
    ASSERT_GT(peu.result.pid, 0);
}

TEST(PEU, ExecUsePath) {
    proc_exec_unit_t peu;
    peu.set_target("echo")
       .push_argument("hi");

    peu.exec_mode   = EXEC_WAIT;
    peu.use_path    = true;
    peu.stdout_mode = STDIO_CAPTURE;

    ::thaumaturgy::yield s = execute(peu);
    ASSERT_EQ(s.is_ok(), true);

    std::string out = read_all_fd(peu.result.stdout_fd);
    ASSERT_EQ(out, "hi\n");
}

TEST(PEU, ExecNoPathFails) {
    proc_exec_unit_t peu;
    peu.set_target("echo");
    peu.exec_mode = EXEC_WAIT;
    peu.use_path  = false;

    ::thaumaturgy::yield s = execute(peu);
    ASSERT_EQ(s.is_ok(), true);
    ASSERT_NE(peu.result.exit_code, 0);
}

// EXEC_REPLACE must be death-tested
TEST(PEU, ExecReplaceFailsGracefully) {
    ASSERT_EXIT({
        proc_exec_unit_t peu;
        peu.set_target("/nonexistent-binary");
        peu.exec_mode = EXEC_REPLACE;
        execute(peu);
        _exit(127); // fallback
    }, ::testing::ExitedWithCode(127), "");
}
