/**
 * @file dotexe.cpp
 * @author Sebastian Czapla (https://github.com/Avarise)
 * @brief SigilVM WINE/Proton Manager (sigilvm-extra package)
 * @version 0.1
 * @date 2025-12-02
 *
 * @copyright Copyright (c) 2026
 *
 * Prefix + Runner manager for running Windows Applications
 */

#include <sigil/platform/process.h>
#include <sigil/platform/compat.h>
#include <sigil/platform/exec.h>
#include <sigil/platform/app.h>
#include <sigil/common.h>
#include <iostream>
#include <vector>

static struct {
    sigil::platform::process_descriptor_t proc_info;
    sigil::platform::app_descriptor_t app_info;
} dotexe_state;

std::vector<sigil::platform::compat_profile_t> profiles = {};
std::vector<sigil::platform::compat_tool_t> runners = {};

::sigil::yield cmd_configure(::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_launcher(::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_special(::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_list(::sigil::platform::cmd_handler_args_t handler_args);

int main(const int argc, const char **argv, const char **envp) {
    sigil::platform::process_initialize(dotexe_state.proc_info, argc, argv, envp);
    sigil::platform::app_initialize(dotexe_state.app_info, dotexe_state.proc_info);

    sigil::platform::command_registry_t reg;
    ::sigil::yield st;

    std::vector<::sigil::platform::command_t> cmds {
        ::sigil::platform::command_t({"configure"}, true,  cmd_configure),
        ::sigil::platform::command_t({"launcher"}, false, cmd_launcher),
        ::sigil::platform::command_t({"list"}, false, cmd_list),
        ::sigil::platform::command_t({}, false, cmd_special),
    };

    st = ::sigil::platform::register_command(reg, cmds);

    if (st.is_failure()) {
        exit(st.code);
    }

    st = ::sigil::platform::dispatch_command(reg, argc, argv);

    return 0;
}

// Configuration: take two arguments, first file/profile second path.exe/profile-name
::sigil::yield cmd_configure(::sigil::platform::cmd_handler_args_t handler_args) {
    sigil::yield st;
    SIGIL_UNUSED(handler_args);

    return st;
}

::sigil::yield cmd_launcher(::sigil::platform::cmd_handler_args_t handler_args) {
    sigil::yield st;
    SIGIL_UNUSED(handler_args);

    return st;
}

::sigil::yield cmd_list(::sigil::platform::cmd_handler_args_t handler_args) {
    sigil::yield st;
    SIGIL_UNUSED(handler_args);

    return st;
}

::sigil::yield cmd_special(::sigil::platform::cmd_handler_args_t handler_args) {
    sigil::yield st;
    std::cout << "Help here yes, mhm?" << std::endl;

    for(auto s : handler_args.switches) {
        std::cout << "Ich bin switch " << s.name << std::endl;
    }

    return st;
}
