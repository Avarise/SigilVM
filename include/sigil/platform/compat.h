#pragma once

/**
 * SigilVM header to manage profiles of compatibility tools for Windows applications.
 * Centered around Proton and Wine + sigil::platform::execute();
 * Profiles, prefixes and tools are stored in ~/.local/sigilvm/wlx64
 * Compatibility Tools installed with Steam are included in Probe.
 */

#include <sigil/status.h>
#include <filesystem>
#include <string>
#include <vector>

namespace sigil::platform {

enum compat_origin_t {
    COMPAT_LOCAL,
    COMPAT_STEAM,
};

struct compat_tool_t {
    std::string name;
    std::filesystem::path path;
    compat_origin_t origin;
};

struct compat_profile_t {
    std::string           file_sha256;
    std::filesystem::path target; // .exe file
    std::filesystem::path runner; // Proton / Wine
    std::filesystem::path prefix; // pfx
    status_t validate();
};

status_t probe_compat_tools(std::vector<compat_tool_t> &out);
status_t probe_compat_profiles(std::vector<compat_profile_t> &out);
status_t load_compat_profile(const std::filesystem::path &src, compat_profile_t &out);
status_t save_compat_profile(const std::filesystem::path &out, compat_profile_t &src);
status_t delete_compat_profile(const std::filesystem::path &src);
status_t run_compat_profile(const compat_profile_t &profile);


}
