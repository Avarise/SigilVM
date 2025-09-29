#include <iostream>
#include <sigil/util/themes.h>
#include <sigil/status.h>
#include <filesystem>
#include <unistd.h>
#include <fcntl.h>

namespace fs = std::filesystem;

static const fs::path sigil_theme_template_dir = "/usr/share/sigilvm/themes/templates";

namespace sigil::util {

status_t list_templates(std::vector<std::string> &out) {
    out.clear();

    if (!fs::exists(sigil_theme_template_dir)) return VM_RESOURCE_MISSING;
    
    for (const auto& entry : fs::directory_iterator(sigil_theme_template_dir)) {
        if (entry.is_directory()) {
            out.push_back(entry.path().filename().string());
        }
    }

    return VM_OK;
}

status_t reload_desktop_components() {
    // Best-effort reload; non-fatal if commands fail.
    int ret = std::system("pgrep -x hyprland >/dev/null 2>&1");
    if (ret != 0) {
        std::cout << "[INFO] Hyprland not detected, skipping reload.\n";
        return VM_DEPENDENCY_MISSING;
    }

    std::cout << "[INFO] Hyprland detected, reloading components...\n";
    // Wallpaper/script
    std::system("~/.config/hypr/scripts/set-default-wallpaper.sh >/dev/null 2>&1");
    // Reload mako, hyprctl
    std::system("makoctl reload >/dev/null 2>&1");
    std::system("hyprctl reload >/dev/null 2>&1");
    // Kvantum
    std::system("kvantummanager --set sigilvm >/dev/null 2>&1");

    // Reload Waybar
    ret = std::system("pkill waybar");

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        setsid();  // Start new session, detach from terminal

        // Redirect both output streams into the void
        int devnull = open("/dev/null", O_RDWR);
        if (devnull >= 0) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            // stdin not needed either
            dup2(devnull, STDIN_FILENO);
            close(devnull);
        }

        execlp("waybar", "waybar", (char*)nullptr);
        _exit(EXIT_FAILURE);  // Only reached if exec fails
    } else if (pid < 0) {
        std::cerr << "[ERROR] Failed to fork waybar process.\n";
    } else {
        std::cout << "[OK] Waybar restarted (PID " << pid << ").\n";
    }

    std::cout << "[OK] Desktop components reload attempted.\n";
    return VM_OK;
}


}






// static sigil::status_t sigil_tools_apply_theme(const std::string& theme) {
//     fs::path theme_dir = sigil_theme_dir / theme;
//     if (!fs::exists(theme_dir)) {
//         std::cerr << "[ERROR] Theme not found: " << theme_dir << std::endl;
//         return sigil::VM_RESOURCE_MISSING;
//     }

//     bool keep_backup = false;
//     std::cout << "[THEME] Applying: " << theme << std::endl;

//     for (const auto& comp : sigil_desktop_components) {
//         fs::path src = theme_dir / comp;
//         fs::path dst = user_config_dir / comp;

//         if (!fs::exists(src)) continue;

//         if (fs::exists(dst)) {
//             if (keep_backup) {
//                 auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
//                 fs::path backup = dst;
//                 backup += ".bak-" + std::to_string(timestamp);
//                 fs::rename(dst, backup);
//                 std::cout << "[BACKUP] " << dst << " → " << backup << std::endl;
//             } else {
//                 std::cout << "[REMOVE] " << dst << std::endl;
//                 fs::remove_all(dst);
//             }
//         }

//         sigil::status_t res = sigil::fs::copy_tree(src, dst);
//         if (res != sigil::VM_OK) {
//             std::cerr << "[ERROR] Failed to copy " << comp << " (status " << res << ")\n";
//             return res;
//         }

//         std::cout << "[OK] " << comp << " applied.\n";
//     }

//     // Handle starship.toml
//     fs::path new_starship = theme_dir / "shell" / "starship.toml";
//     fs::path user_starship = user_config_dir / "starship.toml";
//     if (fs::exists(new_starship)) {
//         if (fs::exists(user_starship)) {
//             if (!sigil::fs::files_identical(new_starship, user_starship)) {
//                 fs::path backup = user_starship;
//                 backup += ".bak";
//                 fs::copy_file(user_starship, backup, fs::copy_options::overwrite_existing);
//                 fs::copy_file(new_starship, user_starship, fs::copy_options::overwrite_existing);
//                 std::cout << "[UPDATE] starship.toml replaced, backup → " << backup << std::endl;
//             } else {
//                 std::cout << "[INFO] starship.toml already matches.\n";
//             }
//         } else {
//             fs::copy_file(new_starship, user_starship);
//             std::cout << "[CREATE] starship.toml created from theme.\n";
//         }
//     }

//     // Kvantum linking (best-effort)
//     fs::path kvantum_theme_src = sigil_theme_dir / theme / "Kvantum";
//     fs::path kvantum_config_dir = user_config_dir / "Kvantum";
//     std::string kvantum_link_name = "sigilvm";
//     fs::path kvantum_link_path = kvantum_config_dir / kvantum_link_name;

//     if (fs::exists(kvantum_theme_src)) {
//         std::error_code ec;
//         fs::create_directories(kvantum_config_dir, ec);
//         if (ec) {
//             std::cerr << "[ERROR] Cannot create Kvantum config dir: " << kvantum_config_dir << std::endl;
//             return sigil::VM_IO_ERROR;
//         }

//         if (fs::exists(kvantum_link_path) || fs::is_symlink(kvantum_link_path))
//             fs::remove_all(kvantum_link_path, ec);

//         std::string kvantum_theme_name = "sigil-" + theme;
//         fs::path kvantum_target = kvantum_theme_src / kvantum_theme_name;

//         if (!fs::exists(kvantum_target)) {
//             std::cerr << "[WARN] Kvantum source theme missing: " << kvantum_target << std::endl;
//             return sigil::VM_RESOURCE_MISSING;
//         }

//         fs::create_directory_symlink(kvantum_target, kvantum_link_path, ec);
//         if (ec) {
//             std::cerr << "[ERROR] Failed to create Kvantum symlink: " << kvantum_link_path << std::endl;
//             return sigil::VM_IO_ERROR;
//         }

//         std::string cmd = "kvantummanager --set " + kvantum_link_name + " >/dev/null 2>&1";
//         std::system(cmd.c_str());
//     }

//     reload_desktop_components_local();
//     std::cout << "[DONE] Theme '" << theme << "' applied successfully.\n";
//     return sigil::VM_OK;
// }