/**
 * @file tools.cpp
 * @author Sebastian Czapla (https://github.com/Avarise)
 * @brief SigilVM Tools (sigilvm-core)
 * @version 0.1
 * @date 2025-12-01
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <sigil/profiling/timer.h>
#include <sigil/platform/device.h>
#include <sigil/network/context.h>
#include <sigil/platform/themes.h>
#include <sigil/render/context.h>
#include <sigil/render/vkutils.h>
#include <sigil/media/context.h>
#include <sigil/realm/context.h>
#include <sigil/data/project.h>
#include <sigil/platform/app.h>
#include <sigil/vm/context.h>
#include <sigil/vm/parser.h>
#include <sigil/math/hash.h>
#include <sigil/status.h>
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
    sigil::vm::context_t *vm_ctx;
} app_context;

// Command handler declarations
sigil::status_t cmd_desktop_reload(const std::vector<std::string> &args);
sigil::status_t cmd_list_themes(const std::vector<std::string> &args);
sigil::status_t cmd_interactive(const std::vector<std::string> &args);
sigil::status_t cmd_build_theme(const std::vector<std::string> &args);
sigil::status_t cmd_set_theme(const std::vector<std::string> &args);
sigil::status_t cmd_unix_time(const std::vector<std::string> &args);
sigil::status_t cmd_hash_dir(const std::vector<std::string> &args);
sigil::status_t cmd_mk_proj(const std::vector<std::string> &args);
sigil::status_t cmd_rm_proj(const std::vector<std::string> &args);
sigil::status_t cmd_ls_proj(const std::vector<std::string> &args);
sigil::status_t cmd_probe(const std::vector<std::string> &args);
sigil::status_t cmd_empty(const std::vector<std::string> &args);
sigil::status_t cmd_flush(const std::vector<std::string> &args);
sigil::status_t cmd_help(const std::vector<std::string> &args);

// Main
int main(const int argc, const char **argv, const char **envp) {
    // First, creation of app descriptor and command registry
    sigil::platform::process_descriptor_t proc(argc, argv, envp);
    sigil::platform::app_descriptor_t app(proc);
    sigil::vm::command_registry_t reg;

    sigil::status_t st = sigil::VM_OK;

    // Now, all the contexts + storing a pointer in global struct
    sigil::vm::context_t vm_ctx(app);
    app_context.vm_ctx = &vm_ctx;

    sigil::media::context_t media_ctx(app);
    app_context.media_ctx = &media_ctx;

    sigil::realm::context_t realm_ctx(app);
    app_context.realm_ctx = &realm_ctx;

    sigil::render::context_t render_ctx(app);
    app_context.render_ctx = &render_ctx;

    sigil::network::context_t network_ctx(app);
    app_context.network_ctx = &network_ctx;

    // Registering commands
    sigil::vm::register_command(reg, sigil::vm::command_t({"theme", "build"}, true, cmd_build_theme));
    sigil::vm::register_command(reg, sigil::vm::command_t({"theme", "set"}, true, cmd_set_theme));
    sigil::vm::register_command(reg, sigil::vm::command_t({"theme", "list"}, true, cmd_list_themes));
    sigil::vm::register_command(reg, sigil::vm::command_t({"project", "create"}, true, cmd_mk_proj));
    sigil::vm::register_command(reg, sigil::vm::command_t({"project", "delete"}, true, cmd_rm_proj));
    sigil::vm::register_command(reg, sigil::vm::command_t({"project", "list"}, false, cmd_ls_proj));
    sigil::vm::register_command(reg, sigil::vm::command_t({"reload", "desktop"}, false, cmd_desktop_reload));
    sigil::vm::register_command(reg, sigil::vm::command_t({"probe"}, true, cmd_probe));
    sigil::vm::register_command(reg, sigil::vm::command_t({"flush"}, false, cmd_flush));
    sigil::vm::register_command(reg, sigil::vm::command_t({"interactive"}, false, cmd_interactive));
    sigil::vm::register_command(reg, sigil::vm::command_t({"hash"}, true, cmd_hash_dir));
    sigil::vm::register_command(reg, sigil::vm::command_t({"help"}, true, cmd_help));
    sigil::vm::register_command(reg, sigil::vm::command_t({"unix-time"}, false, cmd_unix_time));

    // And finally, either run interactive, or parse the arguments as command for tools app
    if (argc == 1) {
        st = cmd_interactive({});
        if (st != sigil::VM_OK) std::cout << "[Error] Interactive mode failed" << std::endl;
    } else {
        st = sigil::vm::dispatch_command(reg, argc, argv);
        if (st != sigil::VM_OK) std::cout << "[Error] Command dispatch failed" << std::endl;
    }

    // Manually clean the global pointer container, so that anything async does not get dangling pointer
    memset(&app_context, 0, sizeof(app_context));
    return 0;
}


/**
 * @brief
 * Empty command; prototyping.
 * @param args
 * @return sigil::status_t
 */
sigil::status_t cmd_empty(const std::vector<std::string> &args) {
    SIGIL_UNUSED(args);
    std::cout << "[ERROR] Empty Handler Called" << std::endl;
    return sigil::VM_FAILED;
}

/**
 * @brief
 * Get Hash of entire directory.
 * Wrapper around hashing function used to detect changes to built themes.
 * @param args
 * @return sigil::status_t
 */
sigil::status_t cmd_hash_dir(const std::vector<std::string> &args) {
    if (args.empty()) {
        std::cout << "[Error] Must provide a path to directory for hashing" << std::endl;
        return sigil::VM_ARG_INVALID;
    }

    sigil::util::timer_t timer;

    timer.start();
    auto hash_val = sigil::math::hash_entire_dir(args.at(0).c_str());
    timer.stop();
    std::cout << args.at(0).c_str() << " -> " << hash_val << " [" << timer.elapsed_milliseconds() << "ms]" << std::endl;

    return sigil::VM_OK;
}

/**
 * @brief
 * Perform SigilVM flush, clear cached data, restart IPC
 * @param args
 * @return sigil::status_t
 */
sigil::status_t cmd_flush(const std::vector<std::string> &args) {
    SIGIL_UNUSED(args);
    std::cout << "Flushing SigilVM" << std::endl;
    return sigil::VM_OK;
}

/**
 * @brief
 * Show list of build themes in ~/.local/share/sigilvm/themes
 * @param args
 * @return sigil::status_t
 */
sigil::status_t cmd_list_themes(const std::vector<std::string> &args) {
    bool show_details = !args.empty() && args.at(0) == "detailed" ? true : false;

    SIGIL_UNUSED(show_details);

    return sigil::VM_OK;
}

/**
 * @brief
 * Wrapper for theme manager, reload Hyprland, Waybar, apply Kvantum Theme
 * @param args
 * @return sigil::status_t
 */
sigil::status_t cmd_desktop_reload(const std::vector<std::string> &args) {
    SIGIL_UNUSED(args);
    return sigil::util::reload_desktop_components();
}

/**
 * @brief
 * Make new project in a SigilVM managed directory ~/Projects
 * @param args
 * @return sigil::status_t
 */
sigil::status_t cmd_mk_proj(const std::vector<std::string> &args) {
    if (args.empty()) {
        std::cout << "Project name cannot be empty.\n";
        return sigil::VM_ARG_INVALID;
    }

    std::string project_name = args.at(0);

    const char* user = std::getenv("USER");
    std::string author = user ? user : "unknown";

    sigil::project_manager_t manager(""); // defaults to ~/Projects
    sigil::status_t status = manager.create_project(project_name, author);

    if (status == sigil::VM_OK) {
        std::cout << "Project \"" << project_name << "\" created successfully by " << author << ".\n";
    } else if (status & sigil::VM_ALREADY_EXISTS) {
        std::cout << "A project with this name already exists.\n";
    } else if (status & sigil::VM_IO_ERROR) {
        std::cout << "I/O error while creating project directories.\n";
    } else if (status & sigil::VM_ARG_INVALID) {
        std::cout << "Invalid project name.\n";
    } else {
        std::cout << "Failed to create project (status code: " << status << ").\n";
    }

    return sigil::VM_OK;
}

/**
 * @brief
 * List projects in a SigilVM managed directory ~/Projects.
 * Requires a project.json in the subdirectory, to be found as valid project.
 * TODO: Move to YAML project file.
 * @param args
 * @return sigil::status_t
 */
sigil::status_t cmd_ls_proj(const std::vector<std::string> &args) {
    SIGIL_UNUSED(args);
    sigil::project_manager_t manager("");  // defaults to ~/Projects
    std::vector<sigil::project_metadata_t> projects;
    if (manager.discover_projects(projects) == sigil::VM_OK) {
        printf("Discovered projects:\n");
        for (auto& p : projects)
            printf("  %s - %s\n", p.name.c_str(), p.description.c_str());
    } else {
        printf("No projects found.\n");
    }
    return sigil::VM_OK;
}

/**
 * @brief
 * Remove a project from SigilVM managed directory ~/Projects.
 * TODO: Implement it, with safeguards to not go outside of the ~/Projects
 * and work only on YAML configured dirs.
 * @param args
 * @return sigil::status_t
 */
sigil::status_t cmd_rm_proj(const std::vector<std::string> &args) {
    SIGIL_UNUSED(args);
    std::cout << "Flushing SigilVM" << std::endl;
    return sigil::VM_OK;
}

/**
 * @brief
 * Probe and print various components of SigilVM.
 * Passing empty args results in full scope scan.
 * TODO: Add more hardware to list of probed components.
 * @param args
 * @return sigil::status_t
 */
sigil::status_t cmd_probe(const std::vector<std::string> &args) {
    if (args.empty()) {
        std::cout << "Starting full SigilVM probe" << std::endl;
        std::cout << "VM Context: " << app_context.vm_ctx << std::endl;
        std::cout << "Media Context: " << app_context.media_ctx << std::endl;
        std::cout << "Network Context: " << app_context.network_ctx << std::endl;
        std::cout << "Render Context: " << app_context.render_ctx << std::endl;
        std::cout << "Realm Context: " << app_context.realm_ctx << std::endl;
    }

    if (!args.empty() && args.at(0) == "gpu") {
        std::vector<sigil::platform::device::gpu_info_t> gpus;
        sigil::status_t status = sigil::render::vk_probe_devices(gpus);
        if (status != sigil::VM_OK) {
            std::cerr << "Failed to enumerate Vulkan devices. Status: " << status << "\n";
            return sigil::VM_FAILED;
        }

        for (const auto& g : gpus) {
            std::cout << "GPU: " << g.name << " ["
                    << g.vram_mb << " MB VRAM]"
                    << " VendorID=" << g.vendor_id
                    << " DeviceID=" << g.device_id
                    << "\n";
        }
    }

    return sigil::VM_OK;
}

/**
 * @brief
 * Deploy a theme from ~/.local/share/sigilvm/themes.
 * Done by copying various files and directories into ~/.config
 * Backup by default, can take extra argument "no-backup", to just overwrite existing
 * @param args
 * @return sigil::status_t
 */
sigil::status_t cmd_set_theme(const std::vector<std::string> &args) {
    if (args.empty()) {
        std::cout << "[Error] Missing name" << std::endl;
        return sigil::VM_ARG_INVALID;
    }

    std::cout << "Setting theme: " << args.at(0) << std::endl;

    return sigil::VM_OK;
}

/**
 * @brief
 * Build one or more themes from templates in /usr/share/sigilvm/themes.
 * Resulting themes will land in ~/.local/share/sigilvm/themes
 * @param args
 * @return sigil::status_t
 */
sigil::status_t cmd_build_theme(const std::vector<std::string> &args) {
    bool build_all = false;
    if (args.empty()) build_all = true;

    if (build_all) {
        std::cout << "Building all themes" << std::endl;
    } else {
        std::cout << "Building theme: " << args.at(0) << std::endl;
    }

    return sigil::VM_OK;
}


/**
 * @brief
 * Print UNIX time
 * @param args
 * @return sigil::status_t
 */
sigil::status_t cmd_unix_time(const std::vector<std::string> &args) {
    SIGIL_UNUSED(args);
    std::cout << "Unix Time: " << sigil::util::unix_time() << std::endl;
    return sigil::VM_OK;
}

/**
 * @brief
 * Start interactive mode, based on NCURSES.
 * @param args
 * @return sigil::status_t
 */
sigil::status_t cmd_interactive(const std::vector<std::string> &args) {
    SIGIL_UNUSED(args);
    std::cout << "Starting interactive mode" << std::endl;
    return sigil::VM_OK;
}

/**
 * @brief
 * Start interactive mode, based on NCURSES.
 * @param args
 * @return sigil::status_t
 */
sigil::status_t cmd_help(const std::vector<std::string> &args) {
    SIGIL_UNUSED(args);

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

    return sigil::VM_OK;
}
