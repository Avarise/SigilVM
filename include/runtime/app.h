#pragma once

#include <filesystem>
#include <functional>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include "../core/debug.h"
#include "../platform/process.h"

static const char*
extract_app_name(const char* path) {
    if (!path) return "";
    const char* slash = std::strrchr(path, '/');
    return (slash ? slash + 1 : path);
}

namespace sigilvm::runtime {

    inline void default_log_info(const std::string &msg) {
        std::cout << msg << std::endl;
    }
    
    inline void default_log_warn(const std::string &msg) {
        std::cout << msg << std::endl;
    }
    
    inline void default_log_error(const std::string &msg) {
        fprintf(stderr, "%s\n", msg.c_str());
    }
    
    inline void default_log_debug(const std::string &msg) {
        sigilvm::dcout << msg << std::endl;
    }

    struct env_kv_t {
        const char *key;
        const char *value;
    };
    
    struct app_descriptor_t {
        const char *app_id;
        const char *app_name;
        const char *dbus_prefix;
        bool initialized = false;
        ::sigilvm::platform::process_descriptor_t process;
        
        std::function<void(const std::string&)> log_info;
        std::function<void(const std::string&)> log_warn;
        std::function<void(const std::string&)> log_error;
        std::function<void(const std::string&)> log_debug;
    
        inline ::sigilvm::status_t initialize(::sigilvm::platform::process_descriptor_t &proc) noexcept {
            ::sigilvm::status_t st;
            if (initialized) {
                return st.set_state(::sigilvm::state::fail);
            }

            if(!proc.initialized) {
                return st.set_state(::sigilvm::state::fail);
            }

            process = proc;
            app_name = extract_app_name((process.argc > 0) ? process.argv[0] : "SIGILVM_INVALID_APP_NAME");
            initialized = true;
            return st;
        }
    };

    inline const char *env_get(::sigilvm::platform::process_descriptor_t const &proc, const char *key) {
        if (!proc.envp) return nullptr;
        for (const char **e = proc.envp; *e; ++e) {
            const char *entry = *e;
            const char *eq = entry;
            while (*eq && *eq != '=') ++eq;
            if (*eq != '=') continue;
            std::size_t klen = static_cast<std::size_t>(eq - entry);
            std::size_t i = 0;
            for (; key[i] && i < klen; ++i) if (key[i] != entry[i]) break;
            if (i == klen && key[i] == '\0') return eq + 1;
        }
        return nullptr;
    }
    
    /* =========================
       Platform root resolution
       ========================= */
    /* These are platform-specific in behavior but exposed as inline API.
       Implementations rely only on env_get + minimal fallback logic. */
    
    inline std::filesystem::path get_config_root(::sigilvm::platform::process_descriptor_t const &p) {
    #if defined(_WIN32)
        if (auto v = env_get(p, "APPDATA")) return v;
        if (auto v = env_get(p, "USERPROFILE")) return std::filesystem::path(v) / "AppData/Roaming";
        return "C:/Temp/sigilvm/config";
    #else
        if (auto v = env_get(p, "XDG_CONFIG_HOME")) return v;
        if (auto h = env_get(p, "HOME")) return std::filesystem::path(h) / ".config";
        return "/tmp/sigilvm/config";
    #endif
    }
    
    inline std::filesystem::path get_cache_root(::sigilvm::platform::process_descriptor_t const &p) {
    #if defined(_WIN32)
        if (auto v = env_get(p, "LOCALAPPDATA")) return v;
        if (auto v = env_get(p, "USERPROFILE")) return std::filesystem::path(v) / "AppData/Local";
        return "C:/Temp/sigilvm/cache";
    #else
        if (auto v = env_get(p, "XDG_CACHE_HOME")) return v;
        if (auto h = env_get(p, "HOME")) return std::filesystem::path(h) / ".cache";
        return "/tmp/sigilvm/cache";
    #endif
    }
    
    inline std::filesystem::path get_data_root(::sigilvm::platform::process_descriptor_t const &p) {
    #if defined(_WIN32)
        if (auto v = env_get(p, "LOCALAPPDATA")) return v;
        if (auto v = env_get(p, "USERPROFILE")) return std::filesystem::path(v) / "AppData/Local";
        return "C:/Temp/sigilvm/data";
    #else
        if (auto v = env_get(p, "XDG_DATA_HOME")) return v;
        if (auto h = env_get(p, "HOME")) return std::filesystem::path(h) / ".local/share";
        return "/tmp/sigilvm/data";
    #endif
    }
    
    inline std::filesystem::path get_state_root(::sigilvm::platform::process_descriptor_t const &p) {
    #if defined(_WIN32)
        if (auto v = env_get(p, "LOCALAPPDATA")) return v;
        if (auto v = env_get(p, "USERPROFILE")) return std::filesystem::path(v) / "AppData/Local";
        return "C:/Temp/sigilvm/state";
    #else
        if (auto v = env_get(p, "XDG_STATE_HOME")) return v;
        if (auto h = env_get(p, "HOME")) return std::filesystem::path(h) / ".local/state";
        return "/tmp/sigilvm/state";
    #endif
    }
    
    /* =========================
       SigilVM base namespaces
       ========================= */
    
    inline std::filesystem::path get_sigilvm_config_root(::sigilvm::platform::process_descriptor_t const &p) { return get_config_root(p) / "sigilvm"; }
    inline std::filesystem::path get_sigilvm_cache_root(::sigilvm::platform::process_descriptor_t const &p) { return get_cache_root(p) / "sigilvm"; }
    inline std::filesystem::path get_sigilvm_data_root(::sigilvm::platform::process_descriptor_t const &p) { return get_data_root(p) / "sigilvm"; }
    inline std::filesystem::path get_sigilvm_state_root(::sigilvm::platform::process_descriptor_t const &p) { return get_state_root(p) / "sigilvm"; }
    
    /* =========================
       App-scoped paths
       ========================= */
    
    inline std::filesystem::path get_local_config(app_descriptor_t const &app) { return get_sigilvm_config_root(app.process) / app.app_name; }
    inline std::filesystem::path get_local_cache(app_descriptor_t const &app) { return get_sigilvm_cache_root(app.process) / app.app_name; }
    inline std::filesystem::path get_local_data(app_descriptor_t const &app) { return get_sigilvm_data_root(app.process) / app.app_name; }
    inline std::filesystem::path get_local_state(app_descriptor_t const &app) { return get_sigilvm_state_root(app.process) / app.app_name; }
    
    /* =========================
       VM-global shared data
       ========================= */
    
    inline std::filesystem::path get_compdata_root(::sigilvm::platform::process_descriptor_t const &p) { return get_sigilvm_data_root(p) / "wlx64"; }
    
    /* =========================
       User-facing directories
       ========================= */
    
    inline std::filesystem::path get_home(::sigilvm::platform::process_descriptor_t const &p) {
    #if defined(_WIN32)
        if (auto v = env_get(p, "USERPROFILE")) return v;
        return "C:/";
    #else
        if (auto v = env_get(p, "HOME")) return v;
        return "/";
    #endif
    }
    
    inline std::filesystem::path get_user_documents(::sigilvm::platform::process_descriptor_t const &p) { return get_home(p) / "Documents"; }
    inline std::filesystem::path get_user_downloads(::sigilvm::platform::process_descriptor_t const &p) { return get_home(p) / "Downloads"; }
    inline std::filesystem::path get_user_pictures(::sigilvm::platform::process_descriptor_t const &p) { return get_home(p) / "Pictures"; }
    inline std::filesystem::path get_user_videos(::sigilvm::platform::process_descriptor_t const &p) { return get_home(p) / "Videos"; }
    inline std::filesystem::path get_user_music(::sigilvm::platform::process_descriptor_t const &p) { return get_home(p) / "Music"; }
    inline std::filesystem::path get_user_workspace(::sigilvm::platform::process_descriptor_t const &p) { return get_home(p) / "Workspace"; }
    
    /* =========================
       Ensure helpers
       ========================= */
    
    inline bool ensure_dir(std::filesystem::path const &p) {
        std::error_code ec;
        if (std::filesystem::exists(p, ec)) return std::filesystem::is_directory(p, ec);
        return std::filesystem::create_directories(p, ec);
    }
    
    /* Base ensures */
    
    inline bool ensure_sigilvm_roots(::sigilvm::platform::process_descriptor_t const &p) {
        return ensure_dir(get_sigilvm_config_root(p))
            && ensure_dir(get_sigilvm_cache_root(p))
            && ensure_dir(get_sigilvm_data_root(p))
            && ensure_dir(get_sigilvm_state_root(p));
    }
    
    /* App ensures */
    
    inline bool ensure_local_all(app_descriptor_t const &app) {
        return ensure_dir(get_local_config(app))
            && ensure_dir(get_local_cache(app))
            && ensure_dir(get_local_data(app))
            && ensure_dir(get_local_state(app));
    }
    
    /* Shared ensures */
    
    inline bool ensure_compdata(::sigilvm::platform::process_descriptor_t const &p) { return ensure_dir(get_compdata_root(p)); }
    
    /* User dirs ensure */
    
    inline bool ensure_user_dirs(::sigilvm::platform::process_descriptor_t const &p) {
        return ensure_dir(get_user_documents(p))
            && ensure_dir(get_user_downloads(p))
            && ensure_dir(get_user_pictures(p))
            && ensure_dir(get_user_videos(p))
            && ensure_dir(get_user_music(p))
            && ensure_dir(get_user_workspace(p));
    }

    // Legacy path helpers from filesystem headers.
    // inline std::string theme_dir_system() {
    //     return "/usr/share/sigilvm/themes";
    // }
    
    // inline std::string theme_dir_local() {
    //     const char* home = std::getenv("HOME");
    //     if (!home)
    //         return "/tmp/sigilvm/themes";   // minimal fallback
    
    //     std::string p;
    //     p.reserve(256);
    //     p += home;
    //     p += "/.local/share/sigilvm/themes";
    //     return p;
    // }
    
    // inline std::filesystem::path get_home_path() {
    //     const char* home = std::getenv("HOME");
    //     if (home && home[0] != '\0') {
    //         return std::filesystem::path(home);
    //     }
    
    //     // --- Fallback path ---
    //     const char* user = std::getenv("USER");
    //     char uid_buf[32];
    
    //     if (!user || user[0] == '\0') {
    //         std::snprintf(uid_buf, sizeof(uid_buf), "uid_%d", getuid());
    //         user = uid_buf;
    //     }
    
    //     std::filesystem::path fallback =
    //         std::filesystem::path("/tmp/home") / user;
    
    //     // Best-effort creation, ignore failure
    //     std::error_code ec;
    //     std::filesystem::create_directories(fallback, ec);
    
    //     return fallback;
    // }
    
    
    // inline std::filesystem::path get_sigilvm_local_path() {
    //     return get_home_path() / ".local" / "share" / "sigilvm";
    // }

} // namespace sigilvm::runtime

