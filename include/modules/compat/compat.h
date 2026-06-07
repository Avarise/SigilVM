#pragma once

/**
 * SigilVM header to manage profiles of compatibility tools for Windows applications.
 * Centered around Proton and Wine + sigil::platform::execute();
 * Profiles, prefixes and tools are stored in ~/.local/sigilvm/wlx64
 * Compatibility Tools installed with Steam are included in Probe.
 */

#include "sigil/platform/app.h"
#include <sigil/common.h>
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
    std::filesystem::path extra;
    bool is_valid();
};

::sigil::yield probe_compat_tools(const ::sigil::platform::app_descriptor_t &app, std::vector<compat_tool_t> &out);
::sigil::yield probe_compat_profiles(const ::sigil::platform::app_descriptor_t &app, std::vector<compat_profile_t> &out);
::sigil::yield load_compat_profile(const ::sigil::platform::app_descriptor_t &app, const std::filesystem::path &src, compat_profile_t &out);
::sigil::yield save_compat_profile(const ::sigil::platform::app_descriptor_t &app, const std::filesystem::path &out, compat_profile_t &src);
::sigil::yield delete_compat_profile(const ::sigil::platform::app_descriptor_t &app, const std::filesystem::path &src);
::sigil::yield run_compat_profile(const ::sigil::platform::app_descriptor_t &app, const compat_profile_t &profile);


}
