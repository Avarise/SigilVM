#include "sigil/platform/filesystem.h"
#include "thaumaturgy/yield.h"
#include <string>
#include <thaumaturgy/thaumaturgy.h>
#include <sigil/platform/desktop.h>
#include <sigil/platform/exec.h>
#include <filesystem>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>

namespace fs = std::filesystem;

static const fs::path sigil_theme_template_dir = "/usr/share/sigilvm/themes/templates";

namespace sigil::desktop {

::thaumaturgy::yield list_templates(std::vector<std::string> &out) {
    out.clear();

    if (!fs::exists(sigil_theme_template_dir))
        return ::thaumaturgy::yield().set_state(thaumaturgy::yield_state::fail);

    for (const auto& entry : fs::directory_iterator(sigil_theme_template_dir)) {
        if (entry.is_directory()) {
            out.push_back(entry.path().filename().string());
        }
    }

    return ::thaumaturgy::yield();
}

::thaumaturgy::yield reload_components() {
    ::sigil::platform::proc_exec_unit_t peu;

    // PEU setup to redirect outputs + wait for process to return
    peu.set_stdio_mode(platform::STDIO_NULL, platform::STDIO_NULL, platform::STDIO_NULL);
    peu.set_exec_mode(platform::EXEC_WAIT);

    // First, check if Hyprland is currently active, and skip if
    peu.set_target("pgrep").push_argument("-x").push_argument("Hyprland");
    platform::execute(peu);

    if (peu.result.exit_code != 0) {
        std::cout << "[INFO] Hyprland not detected, skipping reload." << std::endl;
        return {};
    }

    peu.clean();
    peu.set_target("makoctl").push_argument("reload");
    platform::execute(peu);

    peu.clean();
    peu.set_target("hyprctl").push_argument("reload");
    platform::execute(peu);

    peu.clean();
    peu.set_target("kvantummanager").push_argument("--set").push_argument("sigilvm");
    platform::execute(peu);

    peu.clean();
    peu.set_target("pkill").push_argument("waybar");
    platform::execute(peu);

    peu.clean();
    peu.set_target("~/.config/hypr/scripts/set-default-wallpaper.sh");
    platform::execute(peu);

    peu.clean();
    peu.set_target("waybar").set_exec_mode(platform::EXEC_DETACH);
    platform::execute(peu);

    return ::thaumaturgy::yield();
}

::thaumaturgy::yield deploy_theme_from_path(std::filesystem::path theme_dir) {
    ::thaumaturgy::yield ret;
    
    if (!fs::exists(theme_dir)) {
        std::cerr << "[ERROR] Theme not found: " << theme_dir << std::endl;
        return ret.set_state(thaumaturgy::yield_state::fail);
        // return sigil::VM_RESOURCE_MISSING;
    }

    fs::path user_config_dir = ::sigil::filesystem::get_home_path() / ".config";
    ::std::vector<::std::string> sigil_desktop_components = {
        "alacritty",
        "hypr",
        "mako",
        "waybar",
        "wofi",
    };
    
    bool keep_backup = false;
    std::cout << "[THEME] Applying: " << theme_dir.c_str() << std::endl;

    for (const auto& comp : sigil_desktop_components) {
        fs::path src = theme_dir / comp;
        fs::path dst = user_config_dir / comp;

        if (!fs::exists(src)) continue;

        if (fs::exists(dst)) {
            if (keep_backup) {
                auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
                fs::path backup = dst;
                backup += ".bak-" + std::to_string(timestamp);
                fs::rename(dst, backup);
                std::cout << "[BACKUP] " << dst << " → " << backup << std::endl;
            } else {
                std::cout << "[REMOVE] " << dst << std::endl;
                fs::remove_all(dst);
            }
        }

        ::thaumaturgy::yield res = ::sigil::filesystem::copy_tree(src, dst);
        if (res.is_failure()) {
            std::cerr << "[ERROR] Failed to copy " << comp << " (status " << res.code << ")\n";
            return res;
        }

        std::cout << "[OK] " << comp << " applied.\n";
    }

    // Handle starship.toml
    fs::path new_starship = theme_dir / "shell" / "starship.toml";
    fs::path user_starship = user_config_dir / "starship.toml";
    if (fs::exists(new_starship)) {
        if (fs::exists(user_starship)) {
            if (!sigil::filesystem::files_are_identical(new_starship, user_starship)) {
                fs::path backup = user_starship;
                backup += ".bak";
                fs::copy_file(user_starship, backup, fs::copy_options::overwrite_existing);
                fs::copy_file(new_starship, user_starship, fs::copy_options::overwrite_existing);
                std::cout << "[UPDATE] starship.toml replaced, backup → " << backup << std::endl;
            } else {
                std::cout << "[INFO] starship.toml already matches.\n";
            }
        } else {
            fs::copy_file(new_starship, user_starship);
            std::cout << "[CREATE] starship.toml created from theme.\n";
        }
    }

    // Kvantum linking (best-effort)
    fs::path kvantum_theme_src = theme_dir / "Kvantum";
    fs::path kvantum_config_dir = user_config_dir / "Kvantum";
    std::string kvantum_link_name = "sigilvm";
    fs::path kvantum_link_path = kvantum_config_dir / kvantum_link_name;

    if (fs::exists(kvantum_theme_src)) {
        std::error_code ec;
        fs::create_directories(kvantum_config_dir, ec);
        if (ec) {
            std::cerr << "[ERROR] Cannot create Kvantum config dir: " << kvantum_config_dir << std::endl;
            return ret.set_state(thaumaturgy::yield_state::fail);
            // return sigil::VM_IO_ERROR;
        }

        if (fs::exists(kvantum_link_path) || fs::is_symlink(kvantum_link_path))
            fs::remove_all(kvantum_link_path, ec);

        
        std::string kvantum_theme_name = "sigil-" + std::filesystem::path(theme_dir).filename().string();
;
        fs::path kvantum_target = kvantum_theme_src / kvantum_theme_name;

        if (!fs::exists(kvantum_target)) {
            std::cerr << "[WARN] Kvantum source theme missing: " << kvantum_target << std::endl;
            return ret.set_state(thaumaturgy::yield_state::fail);
            // return sigil::VM_RESOURCE_MISSING;
        }

        fs::create_directory_symlink(kvantum_target, kvantum_link_path, ec);
        if (ec) {
            std::cerr << "[ERROR] Failed to create Kvantum symlink: " << kvantum_link_path << std::endl;
            return ret.set_state(thaumaturgy::yield_state::fail);
            // return sigil::VM_IO_ERROR;
        }

        std::string cmd = "kvantummanager --set " + kvantum_link_name + " >/dev/null 2>&1";
        std::system(cmd.c_str());
    }

    ret |= ::sigil::desktop::reload_components();
    std::cout << "[DONE] Theme '" << theme_dir.string() << "' applied successfully.\n";
    return ret;
}



} // namespace sigil::desktop






// static ::thaumaturgy::yield sigil_tools_apply_theme(const std::string& theme) {

// }
