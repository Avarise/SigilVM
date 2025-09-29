/**
 * @file tools.cpp
 * @author Sebastian Czapla (https://github.com/Avarise)
 * @brief SigilVM Tools (sigilvm-core package)
 * @version 0.1
 * @date 2025-12-01
 *
 * @copyright Copyright (c) 2026
 *
 * Multitool binary for SigilVM
 */

#include <sigil/platform/desktop.h>
#include <sigil/platform/device.h>
#include <sigil/platform/paths.h>
#include <sigil/network/context.h>
#include <sigil/render/context.h>
#include <sigil/render/vkutils.h>
#include <sigil/media/context.h>
#include <sigil/platform/exec.h>
#include <sigil/platform/tty.h>
#include <sigil/platform/app.h>
#include <sigil/utils/crypto.h>
#include <sigil/vm/instance.h>
#include <sigil/platform/fs.h>
#include <sigil/utils/time.h>
#include <sigil/math/hash.h>
#include <sigil/vm/dedup.h>
#include <sigil/common.h>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>

// Struct for pointers of ctxes registered in main, to be used by command handlers
static struct app_context_t {
    sigil::platform::process_descriptor_t proc_info;
    sigil::platform::app_descriptor_t app_info;

    sigil::network::context_t network_ctx;
    sigil::render::context_t render_ctx;
    sigil::media::context_t media_ctx;
} app_context;

// Command handler declarations
::sigil::yield cmd_desktop_reload(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_list_themes(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_interactive(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_build_theme(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_set_theme(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_unix_time(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_hash_dir(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_probe(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_dedup(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_flush(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_help(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_test(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_inspect(const ::sigil::platform::cmd_handler_args_t handler_args);

static std::string build_help_message() {
    return
        "Available commands:\n"
        "\n"
        "  theme build <args...>\n"
        "      Build theme assets.\n"
        "\n"
        "  theme set <args...>\n"
        "      Set active theme.\n"
        "\n"
        "  theme list <args...>\n"
        "      List installed themes.\n"
        "\n"
        "  project create <args...>\n"
        "      Create a new project.\n"
        "\n"
        "  project delete <args...>\n"
        "      Delete an existing project.\n"
        "\n"
        "  project list\n"
        "      List all projects.\n"
        "\n"
        "  reload desktop\n"
        "      Reload desktop definition.\n"
        "\n"
        "  probe <args...>\n"
        "      Probe system information.\n"
        "\n"
        "  flush\n"
        "      Flush caches.\n"
        "\n"
        "  interactive\n"
        "      Start ncurses mode.\n"
        "\n"
        "  hash <args...>\n"
        "      Hash directory contents.\n"
        "\n"
        "  help <command?>\n"
        "      Show this help message.\n";
}

// Main
int main(const int argc, const char **argv, const char **envp) {
    // First, creation of app descriptor and command registry
    sigil::platform::process_initialize(app_context.proc_info, argc, argv, envp);
    sigil::platform::app_initialize(app_context.app_info, app_context.proc_info);

    sigil::platform::command_registry_t reg;

    ::sigil::yield st = {};

    std::vector<::sigil::platform::command_t> cmds {
        ::sigil::platform::command_t({ "theme",   "reload"  }, false, cmd_desktop_reload),
        ::sigil::platform::command_t({ "theme",   "build"   }, true,  cmd_build_theme),
        ::sigil::platform::command_t({ "theme",   "list"    }, true,  cmd_list_themes),
        ::sigil::platform::command_t({ "theme",   "set"     }, true,  cmd_set_theme),

        ::sigil::platform::command_t({ "probe"              }, true,  cmd_probe),
        ::sigil::platform::command_t({ "dedup"              }, true,  cmd_dedup),
        ::sigil::platform::command_t({ "flush"              }, false, cmd_flush),
        ::sigil::platform::command_t({ "interactive"        }, false, cmd_interactive),
        ::sigil::platform::command_t({ "hash"               }, true,  cmd_hash_dir),
        ::sigil::platform::command_t({                      }, false, cmd_help),
        ::sigil::platform::command_t({ "help"               }, true,  cmd_help),
        ::sigil::platform::command_t({ "unix-time"          }, false, cmd_unix_time),
        ::sigil::platform::command_t({ "test"               }, true,  cmd_test),
    };

    st = ::sigil::platform::register_command(reg, cmds);

    if (st.is_failure()) {
        sigil::dcout << "Failed to register command vector: " << st.code << std::endl;
        exit(-1);
    }

    st = sigil::platform::dispatch_command(reg, argc, argv);
    if (st.is_failure()) {
        std::cout << "Failed to run command" << std::endl;
    }

    return st.code;
}

/**
 * @brief
 * Deduplicate a directory
 * @param args
 * @return ::sigil::yield
 */
::sigil::yield cmd_dedup(const ::sigil::platform::cmd_handler_args_t handler_args) {
    sigil::util::timer_t timer;
    ::sigil::yield ret;

    if (handler_args.args.size() < 2) {
        std::cout << "Missing parameters for dedup" << std::endl;
        return ret.set_state(sigil::yield_state::fail);
    }

    std::filesystem::path source = handler_args.args.at(0);
    std::filesystem::path target = handler_args.args.at(1);

    bool dry_run = false;

    for (auto s : handler_args.switches) {
        if (s.name == "--dry-run") dry_run = true;
    }

    timer.start();
    ret |= ::sigil::data::dedup(source, target, dry_run);
    timer.stop();

    std::cout << "Deduplicated in: " << timer.elapsed_milliseconds() << "ms" << std::endl;

    return ret;
}

/**
 * @brief
 * Get Hash of entire directory.
 * Wrapper around hashing function used to detect changes to built themes.
 * @param args
 * @return ::sigil::yield
 */
::sigil::yield cmd_hash_dir(const ::sigil::platform::cmd_handler_args_t handler_args) {
    if (handler_args.args.empty()) {
        std::cout << "[Error] Must provide a path to directory for hashing" << std::endl;
        return ::sigil::yield().set_state(::sigil::yield_state::fail);
    }

    sigil::util::timer_t timer;

    // timer.start();
    // auto hash_val = sigil::math::hash_entire_dir(args.at(0).c_str());
    // timer.stop();
    // std::cout << args.at(0).c_str() << " -> " << hash_val << " [" << timer.elapsed_milliseconds() << "ms]" << std::endl;

    return {};
}

/**
 * @brief
 * Perform SigilVM flush, clear cached data, restart IPC
 * @param args
 * @return ::sigil::yield
 */
::sigil::yield cmd_flush(const ::sigil::platform::cmd_handler_args_t handler_args) {
    SIGIL_UNUSED(handler_args.args);
    std::cout << "Flushing SigilVM" << std::endl;
    return {};
}

/**
 * @brief
 * Show list of build themes in ~/.local/share/sigilvm/themes
 * @param args
 * @return ::sigil::yield
 */
::sigil::yield cmd_list_themes(const ::sigil::platform::cmd_handler_args_t handler_args) {
    bool show_details = !handler_args.args.empty() && handler_args.args.at(0) == "detailed" ? true : false;

    SIGIL_UNUSED(show_details);

    return {};
}

/**
 * @brief
 * Wrapper for theme manager, reload Hyprland, Waybar, apply Kvantum Theme
 * @param args
 * @return ::sigil::yield
 */
::sigil::yield cmd_desktop_reload(const ::sigil::platform::cmd_handler_args_t handler_args) {
    SIGIL_UNUSED(handler_args);
    return sigil::desktop::reload_components();
}


/**
 * @brief
 * Probe and print various components of SigilVM.
 * Passing empty args results in full scope scan.
 * TODO: Add more hardware to list of probed components.
 * @param args
 * @return ::sigil::yield
 */
::sigil::yield cmd_probe(const ::sigil::platform::cmd_handler_args_t handler_args) {
    ::sigil::yield ret;

    if (handler_args.args.empty()) {
        // std::cout << "Starting full SigilVM probe" << std::endl;
        // std::cout << "Media Context: " << app_context.media_ctx << std::endl;
        // std::cout << "Network Context: " << app_context.network_ctx << std::endl;
        // std::cout << "Render Context: " << app_context.render_ctx << std::endl;
        //std::cout << "Realm Context: " << app_context.realm_ctx << std::endl;
    }

    if (!handler_args.args.empty() && handler_args.args.at(0) == "gpu") {
        std::vector<sigil::platform::gpu_info_t> gpus;
        ret = sigil::render::vk_probe_devices(gpus);
        if (ret.is_failure()) {
            std::cerr << "Failed to enumerate Vulkan devices. Status: " << ret.code << "\n";
            return ret;
        }

        for (const auto& g : gpus) {
            std::cout << "GPU: " << g.name << " ["
                    << g.vram_mb << " MB VRAM]"
                    << " VendorID=" << g.vendor_id
                    << " DeviceID=" << g.device_id
                    << "\n";
        }
    }

    return ret;
}

/**
 * @brief
 * Deploy a theme from ~/.local/share/sigilvm/themes.
 * Done by copying various files and directories into ~/.config
 * Backup by default, can take extra argument "no-backup", to just overwrite existing
 * @param args
 * @return ::sigil::yield
 */
::sigil::yield cmd_set_theme
(const ::sigil::platform::cmd_handler_args_t handler_args) {
    if (handler_args.args.empty()) {
        std::cout << "[Error] Missing name" << std::endl;
        return ::sigil::yield().set_state(sigil::yield_state::fail);
    }

    std::string theme_name(handler_args.args.at(0));
    //std::filesystem::path legacy_theme_dir = ::sigil::fs::get_home_path() / ".local" / "share" / "sigilvm" / "themes" / "legacy-themes";
    std::filesystem::path legacy_theme_dir = ::sigil::platform::get_sigilvm_data_root(app_context.proc_info) / "themes" / "legacy-themes";

    std::cout << "Setting theme: " << theme_name << std::endl;

    bool is_legacy_theme = (theme_name == "phoenix"
                        || theme_name == "matrix"
                        || theme_name == "focus"
                        || theme_name == "dark-round"
                        || theme_name == "night");

    if (is_legacy_theme ){
        ::sigil::desktop::deploy_theme_from_path(app_context.app_info, legacy_theme_dir / theme_name);
    }

    return {};
}

/**
 * @brief
 * Build one or more themes from templates in /usr/share/sigilvm/themes.
 * Resulting themes will land in ~/.local/share/sigilvm/themes
 * @param args
 * @return ::sigil::yield
 */
::sigil::yield cmd_build_theme(const ::sigil::platform::cmd_handler_args_t handler_args) {
    ::sigil::yield ret;

    if (handler_args.args.empty()) {
        // Build all, except legacy templates

        std::cout << "Building all themes" << std::endl;
        return ret;
    }

    else if (handler_args.args.at(0) == "legacy") {
        // Build in legacy mode, aka copy hand-made themes
        // from assets/themes/legacy/[matrix|phoenix|focus|night]
        //
        //
        //
        return ret;
    }

    else {
        // Pass build of single theme here
        std::cout << "Building theme: " << handler_args.args.at(0) << std::endl;

        return ret;
    }
}

/**
 * @brief
 * Print UNIX time
 * @param args
 * @return ::sigil::yield
 */
::sigil::yield cmd_unix_time(const ::sigil::platform::cmd_handler_args_t handler_args) {
    SIGIL_UNUSED(handler_args);
    std::cout << "Unix Time: " << sigil::util::unix_time() << std::endl;
    return {};
}

/**
 * @brief
 * Start interactive mode, based on NCURSES.
 * @param args
 * @return ::sigil::yield
 */
::sigil::yield cmd_interactive(const ::sigil::platform::cmd_handler_args_t handler_args) {
    (void)handler_args; // not used yet
    return {};
}

/**
 * @brief
 * Start interactive mode, based on NCURSES.
 * @param args
 * @return ::sigil::yield
 */
::sigil::yield cmd_help(const ::sigil::platform::cmd_handler_args_t handler_args) {
    SIGIL_UNUSED(handler_args);
    std::cout << build_help_message() << std::endl;
    return {};
}

static void fill_random(uint8_t* buf, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        buf[i] = static_cast<uint8_t>(std::rand() & 0xFF);
    }
}

::sigil::yield cmd_test(const ::sigil::platform::cmd_handler_args_t handler_args) {
    sigil::dcout << "[DEBUG] Running Test Command" << std::endl;

    bool xor_test = false;
    bool paths_test = false;

    for (auto ar : handler_args.args) {
        if (ar == "xor_performance") xor_test = true;
        if (ar == "paths") paths_test = true;
    }

    for (auto s : handler_args.switches) {
        auto name = s.name;
        auto value = s.value.has_value() ? s.value.value() : "";
        std::cout << s.name << " " << value << std::endl;
    }

    if (paths_test) {
        auto print = [](const char *label, const std::filesystem::path &p) {
            std::cout << label << ": " << p << "\n";
        };

        std::cout << "=== ROOT PATHS ===\n";
        print("config_root", ::sigil::platform::get_config_root(app_context.proc_info));
        print("cache_root",  ::sigil::platform::get_cache_root(app_context.proc_info));
        print("data_root",   ::sigil::platform::get_data_root(app_context.proc_info));
        print("state_root",  ::sigil::platform::get_state_root(app_context.proc_info));

        std::cout << "\n=== SIGILVM ROOTS ===\n";
        print("sigilvm_config", ::sigil::platform::get_sigilvm_config_root(app_context.proc_info));
        print("sigilvm_cache",  ::sigil::platform::get_sigilvm_cache_root(app_context.proc_info));
        print("sigilvm_data",   ::sigil::platform::get_sigilvm_data_root(app_context.proc_info));
        print("sigilvm_state",  ::sigil::platform::get_sigilvm_state_root(app_context.proc_info));

        std::cout << "\n=== APP LOCAL PATHS ===\n";
        print("local_config", ::sigil::platform::get_local_config(app_context.app_info));
        print("local_cache",  ::sigil::platform::get_local_cache(app_context.app_info));
        print("local_data",   ::sigil::platform::get_local_data(app_context.app_info));
        print("local_state",  ::sigil::platform::get_local_state(app_context.app_info));

        std::cout << "\n=== SHARED ===\n";
        print("compdata", ::sigil::platform::get_compdata_root(app_context.proc_info));

        std::cout << "\n=== USER DIRS ===\n";
        print("home",      ::sigil::platform::get_home(app_context.proc_info));
        print("documents", ::sigil::platform::get_user_documents(app_context.proc_info));
        print("downloads", ::sigil::platform::get_user_downloads(app_context.proc_info));
        print("pictures",  ::sigil::platform::get_user_pictures(app_context.proc_info));
        print("videos",    ::sigil::platform::get_user_videos(app_context.proc_info));
        print("music",     ::sigil::platform::get_user_music(app_context.proc_info));
        print("workspace", ::sigil::platform::get_user_workspace(app_context.proc_info));

        std::cout << "\n=== ENSURE (optional) ===\n";
    }

    if (xor_test) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));

        static const uint8_t key[] = {
            0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x23, 0x34, 0x45,
            0x56, 0x67, 0x78, 0x89, 0xAB, 0xCD, 0xEF, 0x9A,
            0xBC, 0xDE, 0xF0, 0x12, 0x23, 0x34, 0x45, 0x56,
            0x67, 0x78, 0x89, 0xAB, 0xCD, 0xEF, 0xDE, 0xAD,
        };

        // Define the max size once; changing this will affect allocation size
        constexpr size_t MAX_SIZE = 8ull << 30; // 2GB
        uint8_t* buf_simd   = (uint8_t*)std::malloc(MAX_SIZE);
        uint8_t* buf_scalar = (uint8_t*)std::malloc(MAX_SIZE);

        if (!buf_simd || !buf_scalar) {
            if (buf_simd)   std::free(buf_simd);
            if (buf_scalar) std::free(buf_scalar);
            return {};
        }

        // Fill the full SIMD buffer once with random data
        fill_random(buf_simd, MAX_SIZE);
        // Copy to scalar buffer once
        std::memcpy(buf_scalar, buf_simd, MAX_SIZE);

        struct run_t {
            size_t size;
            const char* label;
        };

        const run_t runs[] = {
            { 512ull << 20, "512MB" },
            { 1ull   << 30, "1GB"   },
            { 2ull   << 30, "2GB"   },
            { 4ull   << 30, "4GB"   },
            { 8ull   << 30, "8GB"   },
        };

        sigil::util::timer_t t_scalar;
        sigil::util::timer_t t_simd;
        ::sigil::yield s;

        for (size_t r = 0; r < sizeof(runs) / sizeof(runs[0]); r++) {
            const size_t SIZE = runs[r].size;

            // SIMD-enabled
            t_simd.start();
            s = sigil::utils::xor_encode(
                buf_simd,
                SIZE,
                key,
                sizeof(key),
                /* disable_simd = */ false
            );
            t_simd.stop();

            // Scalar-only
            t_scalar.start();
            s = sigil::utils::xor_encode(
                buf_scalar,
                SIZE,
                key,
                sizeof(key),
                /* disable_simd = */ true
            );
            t_scalar.stop();

            const double simd_ms   = t_simd.elapsed_milliseconds();
            const double scalar_ms = t_scalar.elapsed_milliseconds();

            std::cout
                << "[ XOR PERF ] "
                << runs[r].label
                << " | SIMD: "   << simd_ms   << " ms"
                << " | Scalar: " << scalar_ms << " ms"
                << std::endl;
        }

        std::free(buf_simd);
        std::free(buf_scalar);
    }

    return {};
}
