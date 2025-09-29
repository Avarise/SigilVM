#pragma once

/**
 * file: include/sigil/platform/paths.h
 *
 * Stateless path resolution + ensure helpers for SigilVM.
 *
 * Design:
 * - pure getters (no I/O)
 * - explicit ensure_* for filesystem materialization
 * - app-scoped paths derived from base paths
 * - environment sourced from process_descriptor_t
 */

#include <filesystem>
#include <string>
#include <sigil/platform/app.h>

namespace sigil::platform {

/* =========================
   Internal env helpers
   ========================= */

struct env_kv_t {
    const char *key;
    const char *value;
};

inline const char *env_get(process_descriptor_t const &proc, const char *key) {
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

inline std::filesystem::path get_config_root(process_descriptor_t const &p) {
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

inline std::filesystem::path get_cache_root(process_descriptor_t const &p) {
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

inline std::filesystem::path get_data_root(process_descriptor_t const &p) {
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

inline std::filesystem::path get_state_root(process_descriptor_t const &p) {
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

inline std::filesystem::path get_sigilvm_config_root(process_descriptor_t const &p) { return get_config_root(p) / "sigilvm"; }
inline std::filesystem::path get_sigilvm_cache_root(process_descriptor_t const &p) { return get_cache_root(p) / "sigilvm"; }
inline std::filesystem::path get_sigilvm_data_root(process_descriptor_t const &p) { return get_data_root(p) / "sigilvm"; }
inline std::filesystem::path get_sigilvm_state_root(process_descriptor_t const &p) { return get_state_root(p) / "sigilvm"; }

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

inline std::filesystem::path get_compdata_root(process_descriptor_t const &p) { return get_sigilvm_data_root(p) / "wlx64"; }

/* =========================
   User-facing directories
   ========================= */

inline std::filesystem::path get_home(process_descriptor_t const &p) {
#if defined(_WIN32)
    if (auto v = env_get(p, "USERPROFILE")) return v;
    return "C:/";
#else
    if (auto v = env_get(p, "HOME")) return v;
    return "/";
#endif
}

inline std::filesystem::path get_user_documents(process_descriptor_t const &p) { return get_home(p) / "Documents"; }
inline std::filesystem::path get_user_downloads(process_descriptor_t const &p) { return get_home(p) / "Downloads"; }
inline std::filesystem::path get_user_pictures(process_descriptor_t const &p) { return get_home(p) / "Pictures"; }
inline std::filesystem::path get_user_videos(process_descriptor_t const &p) { return get_home(p) / "Videos"; }
inline std::filesystem::path get_user_music(process_descriptor_t const &p) { return get_home(p) / "Music"; }
inline std::filesystem::path get_user_workspace(process_descriptor_t const &p) { return get_home(p) / "Workspace"; }

/* =========================
   Ensure helpers
   ========================= */

inline bool ensure_dir(std::filesystem::path const &p) {
    std::error_code ec;
    if (std::filesystem::exists(p, ec)) return std::filesystem::is_directory(p, ec);
    return std::filesystem::create_directories(p, ec);
}

/* Base ensures */

inline bool ensure_sigilvm_roots(process_descriptor_t const &p) {
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

inline bool ensure_compdata(process_descriptor_t const &p) { return ensure_dir(get_compdata_root(p)); }

/* User dirs ensure */

inline bool ensure_user_dirs(process_descriptor_t const &p) {
    return ensure_dir(get_user_documents(p))
        && ensure_dir(get_user_downloads(p))
        && ensure_dir(get_user_pictures(p))
        && ensure_dir(get_user_videos(p))
        && ensure_dir(get_user_music(p))
        && ensure_dir(get_user_workspace(p));
}

} // namespace sigil::platform
