#include "sigil/debug.h"
#include "sigil/platform/exec.h"
#include "thaumaturgy/yield.h"
#include <cstring>
#include <iostream>
#include <sigil/platform/filesystem.h>
#include <sigil/platform/compat.h>
#include <thaumaturgy/thaumaturgy.h>
#include <filesystem>
#include <sys/stat.h>

static bool
path_is_wine(const std::filesystem::path& runner)
{
    return runner.filename() == "wine";
}

static bool
path_is_proton(const std::filesystem::path& runner)
{
    return std::filesystem::is_directory(runner)
        && std::filesystem::exists(runner / "proton");
}

static bool hash_is_matching(const std::filesystem::path &file_path,
                             const std::string &expected_sha256) {
    sigil::platform::proc_exec_unit_t peu{};
    peu.set_target("sha256sum");
    peu.push_argument(file_path);
    peu.exec_mode = sigil::platform::EXEC_WAIT;
    // Capture output instead of allowing it to hit our main console out
    peu.set_stdio_mode(sigil::platform::STDIO_CAPTURE, sigil::platform::STDIO_CAPTURE, sigil::platform::STDIO_CAPTURE);

    ::thaumaturgy::yield s = execute(peu);
    if (s.is_failure())
        return false;

    if (peu.result.exit_code != 0)
        return false;

    if (peu.result.stdout_fd < 0)
        return false;

    // Determine output size
    struct stat st;
    if (fstat(peu.result.stdout_fd, &st) != 0)
        return false;

    if (st.st_size < 64)
        return false;

    std::string output;
    output.resize(st.st_size);

    ssize_t rd = pread(
        peu.result.stdout_fd,
        output.data(),
        output.size(),
        0
    );

    if (rd < 64)
        return false;

    // sha256sum: first 64 chars are the hash
    std::string actual_hash = output.substr(0, 64);

    // normalize expected hash (trim whitespace)
    std::string expected = expected_sha256;

    expected.erase(0, expected.find_first_not_of(" \t\r\n"));
    expected.erase(expected.find_last_not_of(" \t\r\n") + 1);

    return actual_hash == expected;
}


namespace sigil::platform {

::thaumaturgy::yield probe_compat_tools(std::vector<compat_tool_t> &out) {
    // Scan the SigilVM Compat directory
    std::filesystem::path sigil_compat = sigil::filesystem::get_sigilvm_local_path() / "wlx64" / "runners";

    if (std::filesystem::exists(sigil_compat)) {
        for (auto& e : std::filesystem::directory_iterator(sigil_compat)) {
            if (!e.is_directory())
                continue;

            sigil::platform::compat_tool_t t;
            t.name   = e.path().filename().string();
            t.path   = e.path().string();
            t.origin = COMPAT_LOCAL;
            out.push_back(t);
        }
    }

    // Scan the Steam Compat directory
    std::filesystem::path steam_compat = sigil::filesystem::get_home_path() / ".local/share/Steam/compatibilitytools.d";

    if (std::filesystem::exists(steam_compat)) {
        for (auto& e : std::filesystem::directory_iterator(steam_compat)) {
            if (!e.is_directory())
                continue;

            sigil::platform::compat_tool_t t;
            t.name   = e.path().filename().string();
            t.path   = e.path().string();
            t.origin = COMPAT_STEAM;
            out.push_back(t);
        }
    }

    // Detect WINE binary
    if (sigil::filesystem::binary_exists_in_path("wine")) {
        compat_tool_t wine;
        wine.name   = "Wine";
        wine.path   = "wine";
        wine.origin = COMPAT_LOCAL;
        out.push_back(wine);
    }

    return ::thaumaturgy::yield();
}


bool compat_profile_t::is_valid() {
    if (!std::filesystem::exists(this->target))
        return false;

    if (!std::filesystem::exists(this->runner))
        return false;

    if (!std::filesystem::exists(this->prefix))
        return false;

    return true;
}

::thaumaturgy::yield load_compat_profile(const std::filesystem::path &src, compat_profile_t &out) {
    ::thaumaturgy::yield ret;
    
    FILE* f = fopen(src.c_str(), "r");
    if (!f)
        return ret.set_state(thaumaturgy::yield_state::fail);
        // return sigil::VM_RESOURCE_MISSING;

    char line[512];

    while (fgets(line, sizeof(line), f)) {
        std::string line_str(line);

        //sigil::dcout << line_str << std::endl;

        // skip empty lines
        if (line_str.empty())
            continue;

        // find the first colon
        auto colon_pos = line_str.find(':');
        if (colon_pos == std::string::npos)
            continue; // no key-value separator

        std::string key = line_str.substr(0, colon_pos);
        std::string val = line_str.substr(colon_pos + 1);

        // trim whitespace from key
        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);

        // trim whitespace from val
        val.erase(0, val.find_first_not_of(" \t\r\n"));
        val.erase(val.find_last_not_of(" \t\r\n") + 1);

        //sigil::dcout << "Parsing key: " << key << " with value: " << val << std::endl;

        // assign values
        if (key == "target")
            out.target = val;
        else if (key == "runner")
            out.runner = val;
        else if (key == "prefix")
            out.prefix = val;
        else if (key == "sha256") {
            if (!hash_is_matching(out.target, val)) {
                sigil::dcout << "[ERROR] Hash mismatch when opening a profile" << std::endl;
                return ret.set_state(thaumaturgy::yield_state::fail);
                // return sigil::VM_INVALID_STATE;
            } else {
                out.file_sha256 = val;
            }
        }
    }

    fclose(f);
    
    if (out.is_valid())
        ret.set_state(thaumaturgy::yield_state::fail); 
        
    return ret;
}


::thaumaturgy::yield
run_compat_profile(const compat_profile_t& profile) {
    compat_profile_t p = profile;
    ::thaumaturgy::yield ret;
    
    if (!p.is_valid()) {
        return ret.set_state(thaumaturgy::yield_state::fail);
    }
    
    sigil::platform::proc_exec_unit_t peu;
    peu.set_exec_mode(EXEC_WAIT);

    if (path_is_wine(profile.runner)) {
        peu.export_var("WINEPREFIX", profile.prefix);
        peu.set_target(profile.runner);
        peu.push_argument(profile.target);
    }
    else if (path_is_proton(profile.runner)) {
        peu.set_target(profile.runner / "proton");
        peu.export_var("PROTON_LOG", "1");
        peu.export_var("PROTON_USE_WINED3D", "1");
        peu.export_var("PROTON_NO_ESYNC", "1");
        peu.export_var("PROTON_NO_FSYNC", "1");
        peu.export_var("WINEDLLOVERRIDES", "mscoree=b;msi=b");
        peu.export_var("STEAM_COMPAT_CLIENT_INSTALL_PATH", sigil::filesystem::get_home_path() / ".steam" / "steam");
        peu.export_var("STEAM_COMPAT_DATA_PATH", profile.prefix);
        peu.push_argument("run").push_argument(profile.target);
    }
    else {
        return ret.set_state(thaumaturgy::yield_state::fail);
    }

    sigil::platform::execute(peu);

    return ret;
}


::thaumaturgy::yield probe_compat_profiles(std::vector<compat_profile_t> &out) {
    out.clear();
    ::thaumaturgy::yield ret;

    std::filesystem::path profiles_dir =
        sigil::filesystem::get_sigilvm_local_path()
        / "wlx64"
        / "profiles";

    if (!std::filesystem::exists(profiles_dir))
        return ret.set_state(thaumaturgy::yield_state::fail);
        // return sigil::VM_OK;
        
    if (!std::filesystem::is_directory(profiles_dir))
        return ret.set_state(thaumaturgy::yield_state::fail);
        // return sigil::VM_ARG_INVALID;

    for (const auto &e : std::filesystem::directory_iterator(profiles_dir)) {
        if (!e.is_regular_file())
            continue;

        compat_profile_t profile{};
        ::thaumaturgy::yield s = load_compat_profile(e.path(), profile);

        if (s.is_failure()) {
            sigil::dcout << "[DEBUG] Skipping invalid profile" << std::endl;
            continue; // skip invalid / unreadable profiles
        }

        out.push_back(profile);
    }

    return ret;
}



/*::thaumaturgy::yieldsave_profile(const std::filesystem::path &out, compat_profile_t &src) {
    fs::create_directories(profiles_dir());

    fs::path path = profiles_dir() / name;
    path += ".yaml";

    FILE* f = fopen(path.c_str(), "w");
    if (!f)
        return sigil::VM_ERR_IO;

    fprintf(f, "version: 1\n");
    fprintf(f, "target: %s\n", p.target.c_str());
    fprintf(f, "runner: %s\n", p.runner.c_str());
    fprintf(f, "prefix: %s\n", p.prefix.c_str());
    fprintf(f, "sha256: %s\n", sha256_to_hex(p.file_sha256).c_str());

    fclose(f);
    return sigil::VM_OK;


    }*/

} // namespace sigil::compat
