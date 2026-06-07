/**
 * @file player.cpp
 * @author Sebastian Czapla (https://github.com/Avarise)
 * @brief SigilVM Media Player (sigilvm-desktop package)
 * @version 0.1
 * @date 2025-12-02
 *
 * @copyright Copyright (c) 2026
 * 
 * MPRIS-compatible media player based on FFMPEG
 */

#include <sigil/platform/process.h>
#include <sigil/platform/exec.h>
#include <sigil/media/context.h>
#include <sigil/platform/app.h>
#include <sigil/common.h>
#include <iostream>
#include <vector>

static struct app_context_t {
    sigil::platform::process_descriptor_t proc_info;
    sigil::platform::app_descriptor_t app_info;
} player_state;

// Command handler declarations
::sigil::yield cmd_previous(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_playlist(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_unpause(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_pause(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_stop(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_next(const ::sigil::platform::cmd_handler_args_t handler_args);
::sigil::yield cmd_play(const ::sigil::platform::cmd_handler_args_t handler_args);

int main(const int argc, const  char **argv, const char **envp) {
    // First, creation of app descriptor and command registry
    sigil::platform::process_initialize(player_state.proc_info, argc, argv, envp);
    sigil::platform::app_initialize(player_state.app_info, player_state.proc_info);

    ::sigil::platform::command_registry_t reg;
    ::sigil::yield st = {};

    ::std::vector<::sigil::platform::command_t> cmds = {
        sigil::platform::command_t({"previous"}, false, cmd_previous),
        sigil::platform::command_t({"playlist"}, true, cmd_playlist),
        sigil::platform::command_t({"pause"}, false, cmd_pause),
        sigil::platform::command_t({"stop"}, false, cmd_stop),
        sigil::platform::command_t({"next"}, false, cmd_next),
        sigil::platform::command_t({"play"}, true, cmd_play),
    };
    
    st = ::sigil::platform::register_command(reg, cmds);
    if (st.is_failure()) {
        exit(st.code);
    }
    
    std::cout << "Media player not implemented yet" << std::endl;
    return 0;
}


::sigil::yield cmd_previous(const ::sigil::platform::cmd_handler_args_t handler_args) {
    SIGIL_UNUSED(handler_args);

    return {};
}

::sigil::yield cmd_playlist(const ::sigil::platform::cmd_handler_args_t handler_args){
    SIGIL_UNUSED(handler_args);

    return {};
}


::sigil::yield cmd_unpause(const ::sigil::platform::cmd_handler_args_t handler_args){
    SIGIL_UNUSED(handler_args);

    return {};
}


::sigil::yield cmd_pause(const ::sigil::platform::cmd_handler_args_t handler_args){
    SIGIL_UNUSED(handler_args);

    return {};
}


::sigil::yield cmd_stop(const ::sigil::platform::cmd_handler_args_t handler_args) {
    SIGIL_UNUSED(handler_args);

    return {};
}


::sigil::yield cmd_next(const ::sigil::platform::cmd_handler_args_t handler_args) {
    SIGIL_UNUSED(handler_args);

    return {};
}


::sigil::yield cmd_play(const ::sigil::platform::cmd_handler_args_t handler_args) {
    SIGIL_UNUSED(handler_args);

    return {};
}

/**
 * Abstractions we need for the player (some implemented already)
 * VM Context + Command parser -> High level control via apps and desktop components
 * Audio Device(s) -> stored in the context, maybe as vector of found devices?
 * Playback controller with commands to play/pause/stop/loop toggle etc
 * Playlist manager, holds vector of playlists, while each playlist, after being loaded is vector of track data (file path etc)
 * 
 * All traffic should be routed 
 */