/**
 * @file tools.cpp
 * @author Sebastian Czapla (https://github.com/Avarise)
 * @brief SigilVM Tools (sigilvm-core)
 * @version 0.1
 * @date 2025-12-01
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <sigil/platform/filesystem.h>
#include <thaumaturgy/thaumaturgy.h>
#include <sigil/platform/desktop.h>
#include <sigil/profiling/timer.h>
#include <sigil/platform/device.h>
#include <sigil/network/context.h>
#include <sigil/render/context.h>
#include <sigil/render/vkutils.h>
#include <sigil/media/context.h>
#include <sigil/platform/exec.h>
#include <sigil/realm/context.h>
#include <sigil/data/project.h>
#include <sigil/platform/app.h>
#include <sigil/data/dedup.h>
#include <sigil/math/hash.h>
#include <sigil/macros.h>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>

// Struct for pointers of ctxes registered in main, to be used by command handlers
static struct app_context_t {
    sigil::network::context_t *network_ctx;
    sigil::render::context_t *render_ctx;
    sigil::media::context_t *media_ctx;
    sigil::realm::context_t *realm_ctx;
} app_context;

// Command handler declarations
::thaumaturgy::yield cmd_desktop_reload(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_list_themes(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_interactive(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_build_theme(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_set_theme(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_unix_time(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_hash_dir(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_mk_proj(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_rm_proj(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_ls_proj(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_probe(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_dedup(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_empty(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_flush(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_help(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_test(const ::sigil::platform::cmd_handler_args handler_args);

// Main
int main(const int argc, const char **argv, const char **envp) {
    // First, creation of app descriptor and command registry
    sigil::platform::process_descriptor_t proc(argc, argv, envp);
    sigil::platform::app_descriptor_t app(proc);
    sigil::platform::command_registry_t reg;

    ::thaumaturgy::yield st = {};

    sigil::media::context_t media_ctx(app);
    app_context.media_ctx = &media_ctx;

    sigil::realm::context_t realm_ctx(app);
    app_context.realm_ctx = &realm_ctx;

    sigil::render::context_t render_ctx(app);
    app_context.render_ctx = &render_ctx;

    sigil::network::context_t network_ctx(app);
    app_context.network_ctx = &network_ctx;

    std::vector<::sigil::platform::command_t> cmds {
        ::sigil::platform::command_t({ "theme",   "reload"  }, false, cmd_desktop_reload),
        ::sigil::platform::command_t({ "theme",   "build"   }, true,  cmd_build_theme),
        ::sigil::platform::command_t({ "theme",   "list"    }, true,  cmd_list_themes),
        ::sigil::platform::command_t({ "theme",   "set"     }, true,  cmd_set_theme),

        ::sigil::platform::command_t({ "project", "create"  }, true,  cmd_mk_proj),
        ::sigil::platform::command_t({ "project", "delete"  }, true,  cmd_rm_proj),
        ::sigil::platform::command_t({ "project", "list"    }, false, cmd_ls_proj),


        ::sigil::platform::command_t({ "probe"              }, true,  cmd_probe),
        ::sigil::platform::command_t({ "dedup"              }, true,  cmd_dedup),
        ::sigil::platform::command_t({ "flush"              }, false, cmd_flush),
        ::sigil::platform::command_t({ "interactive"        }, false, cmd_interactive),
        ::sigil::platform::command_t({ "hash"               }, true,  cmd_hash_dir),
        ::sigil::platform::command_t({ "help"               }, true,  cmd_help),
        ::sigil::platform::command_t({ "unix-time"          }, false, cmd_unix_time),
        ::sigil::platform::command_t({ "test"               }, true,  cmd_test),
    };

    st = ::sigil::platform::register_command(reg, cmds);

    if (st.is_failure()) {
        exit(-1);
    }


    // And finally, either run interactive, or parse the arguments as command for tools app
    if (argc == 1) {
        st = cmd_interactive({});
        if (st.is_failure()) std::cout << "[Error] Interactive mode failed" << std::endl;
    } else {
        st = sigil::platform::dispatch_command(reg, argc, argv);
        if (st.is_failure()) std::cout << "[Error] Command dispatch failed" << std::endl;
    }

    // Manually clean the global pointer container, so that anything async does not get dangling pointer
    memset(&app_context, 0, sizeof(app_context));
    return 0;
}

/**
 * @brief
 * Empty command; prototyping.
 * @param args
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield cmd_empty(const ::sigil::platform::cmd_handler_args handler_args) {
    SIGIL_UNUSED(handler_args);
    std::cout << "[ERROR] Empty Handler Called" << std::endl;
    return ::thaumaturgy::yield().set_state(::thaumaturgy::yield_state::fail);
}

/**
 * @brief
 * Deduplicate a directory
 * @param args
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield cmd_dedup(const ::sigil::platform::cmd_handler_args handler_args) {
    ::thaumaturgy::yield ret;

    if (handler_args.args.size() < 2)
        return ret.set_state(thaumaturgy::yield_state::fail);

    std::filesystem::path source = handler_args.args.at(0);
    std::filesystem::path target = handler_args.args.at(1);

    bool dry_run = false;

    for (auto s : handler_args.switches) {
        if (s == "--dry-run") dry_run = true;
    }

    ret |= ::sigil::data::dedup(source, target, dry_run);

    return ret;
}


/**
 * @brief
 * Get Hash of entire directory.
 * Wrapper around hashing function used to detect changes to built themes.
 * @param args
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield cmd_hash_dir(const ::sigil::platform::cmd_handler_args handler_args) {
    if (handler_args.args.empty()) {
        std::cout << "[Error] Must provide a path to directory for hashing" << std::endl;
        return ::thaumaturgy::yield().set_state(::thaumaturgy::yield_state::fail);
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
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield cmd_flush(const ::sigil::platform::cmd_handler_args handler_args) {
    SIGIL_UNUSED(handler_args.args);
    std::cout << "Flushing SigilVM" << std::endl;
    return {};
}

/**
 * @brief
 * Show list of build themes in ~/.local/share/sigilvm/themes
 * @param args
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield cmd_list_themes(const ::sigil::platform::cmd_handler_args handler_args) {
    bool show_details = !handler_args.args.empty() && handler_args.args.at(0) == "detailed" ? true : false;

    SIGIL_UNUSED(show_details);

    return {};
}

/**
 * @brief
 * Wrapper for theme manager, reload Hyprland, Waybar, apply Kvantum Theme
 * @param args
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield cmd_desktop_reload(const ::sigil::platform::cmd_handler_args handler_args) {
    SIGIL_UNUSED(handler_args);
    return sigil::desktop::reload_components();
}

/**
 * @brief
 * Make new project in a SigilVM managed directory ~/Projects
 * @param args
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield cmd_mk_proj(const ::sigil::platform::cmd_handler_args handler_args) {
    if (handler_args.args.empty()) {
        std::cout << "Project name cannot be empty.\n";
        return ::thaumaturgy::yield().set_state(thaumaturgy::yield_state::partial);
    }

    std::string project_name = handler_args.args.at(0);

    const char* user = std::getenv("USER");
    std::string author = user ? user : "unknown";

    // sigil::project_manager_t manager(""); // defaults to ~/Projects
    // ::thaumaturgy::yield status = manager.create_project(project_name, author);

    // if (status == sigil::platform_OK) {
    //     std::cout << "Project \"" << project_name << "\" created successfully by " << author << ".\n";
    // } else if (status & sigil::platform_ALREADY_EXISTS) {
    //     std::cout << "A project with this name already exists.\n";
    // } else if (status & sigil::platform_IO_ERROR) {
    //     std::cout << "I/O error while creating project directories.\n";
    // } else if (status & sigil::platform_ARG_INVALID) {
    //     std::cout << "Invalid project name.\n";
    // } else {
    //     std::cout << "Failed to create project (status code: " << status << ").\n";
    // }

    return {};
}

/**
 * @brief
 * List projects in a SigilVM managed directory ~/Projects.
 * Requires a project.json in the subdirectory, to be found as valid project.
 * TODO: Move to YAML project file.
 * @param args
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield cmd_ls_proj(const ::sigil::platform::cmd_handler_args handler_args) {
    SIGIL_UNUSED(handler_args);
    // // sigil::project_manager_t manager("");  // defaults to ~/Projects
    // std::vector<sigil::project_metadata_t> projects;
    // if (manager.discover_projects(projects) == sigil::platform_OK) {
    //     printf("Discovered projects:\n");
    //     for (auto& p : projects)
    //         printf("  %s - %s\n", p.name.c_str(), p.description.c_str());
    // } else {
    //     printf("No projects found.\n");
    // }
    return {};
}

/**
 * @brief
 * Remove a project from SigilVM managed directory ~/Projects.
 * TODO: Implement it, with safeguards to not go outside of the ~/Projects
 * and work only on YAML configured dirs.
 * @param args
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield cmd_rm_proj(const ::sigil::platform::cmd_handler_args handler_args) {
    SIGIL_UNUSED(handler_args);
    std::cout << "Removing Project" << std::endl;
    return {};
}

/**
 * @brief
 * Probe and print various components of SigilVM.
 * Passing empty args results in full scope scan.
 * TODO: Add more hardware to list of probed components.
 * @param args
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield cmd_probe(const ::sigil::platform::cmd_handler_args handler_args) {
    ::thaumaturgy::yield ret;

    if (handler_args.args.empty()) {
        std::cout << "Starting full SigilVM probe" << std::endl;
        std::cout << "Media Context: " << app_context.media_ctx << std::endl;
        std::cout << "Network Context: " << app_context.network_ctx << std::endl;
        std::cout << "Render Context: " << app_context.render_ctx << std::endl;
        std::cout << "Realm Context: " << app_context.realm_ctx << std::endl;
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
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield cmd_set_theme(const ::sigil::platform::cmd_handler_args handler_args) {
    if (handler_args.args.empty()) {
        std::cout << "[Error] Missing name" << std::endl;
        return ::thaumaturgy::yield().set_state(thaumaturgy::yield_state::fail);
    }

    std::string theme_name(handler_args.args.at(0));
    std::filesystem::path legacy_theme_dir = ::sigil::filesystem::get_home_path() / ".local" / "share" / "sigilvm" / "themes" / "legacy-themes";

    std::cout << "Setting theme: " << theme_name << std::endl;

    bool is_legacy_theme = (theme_name == "phoenix"
                        || theme_name == "matrix"
                        || theme_name == "focus"
                        || theme_name == "dark-round"
                        || theme_name == "night");

    if (is_legacy_theme ){
        ::sigil::desktop::deploy_theme_from_path(legacy_theme_dir / theme_name);
    }

    return {};
}

/**
 * @brief
 * Build one or more themes from templates in /usr/share/sigilvm/themes.
 * Resulting themes will land in ~/.local/share/sigilvm/themes
 * @param args
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield cmd_build_theme(const ::sigil::platform::cmd_handler_args handler_args) {
    ::thaumaturgy::yield ret;

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
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield cmd_unix_time(const ::sigil::platform::cmd_handler_args handler_args) {
    SIGIL_UNUSED(handler_args);
    std::cout << "Unix Time: " << sigil::util::unix_time() << std::endl;
    return {};
}

/**
 * @brief
 * Start interactive mode, based on NCURSES.
 * @param args
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield cmd_interactive(const ::sigil::platform::cmd_handler_args handler_args) {
    SIGIL_UNUSED(handler_args);
    std::cout << "Starting interactive mode" << std::endl;
    return {};
}

/**
 * @brief
 * Start interactive mode, based on NCURSES.
 * @param args
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield cmd_help(const ::sigil::platform::cmd_handler_args handler_args) {
    SIGIL_UNUSED(handler_args);

    std::string help_msg = {
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
        "      Show this help message.\n"
    };

    std::cout << help_msg << std::endl;

    return {};
}



::thaumaturgy::yield cmd_test(const ::sigil::platform::cmd_handler_args handler_args) {
    for (auto ar : handler_args.args) {
        std::cout << ar << std::endl;
    }

    for (auto s : handler_args.switches) {
        std::cout << s << std::endl;
    }

    return {};
}
