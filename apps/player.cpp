/**
 * @file player.cpp
 * @author Sebastian Czapla (https://github.com/Avarise)
 * @brief SigilVM Media Player (sigilvm-desktop)
 * @version 0.1
 * @date 2025-12-02
 *
 * @copyright Copyright (c) 2026
 * 
 * MPRIS-compatible media player based on FFMPEG
 */

#include <thaumaturgy/thaumaturgy.h>
#include <sigil/platform/process.h>
#include <sigil/platform/exec.h>
#include <sigil/media/context.h>
#include <sigil/platform/app.h>
#include <sigil/macros.h>
#include <iostream>
#include <vector>

static struct app_context_t {
    sigil::media::context_t *media_ctx;
} app_context;

// Command handler declarations
::thaumaturgy::yield cmd_previous(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_playlist(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_unpause(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_pause(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_stop(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_next(const ::sigil::platform::cmd_handler_args handler_args);
::thaumaturgy::yield cmd_play(const ::sigil::platform::cmd_handler_args handler_args);

int main(const int argc, const  char **argv, const char **envp) {
    // First, creation of app descriptor and command registry
    ::sigil::platform::process_descriptor_t proc(argc, argv, envp);
    ::sigil::platform::app_descriptor_t app(proc);
    ::sigil::platform::command_registry_t reg;
    ::thaumaturgy::yield st = {};

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
    
    sigil::media::context_t media_ctx(app);
    app_context.media_ctx = &media_ctx;

    std::cout << "Media player not implemented yet" << std::endl;
    return 0;
}


::thaumaturgy::yield cmd_previous(const ::sigil::platform::cmd_handler_args handler_args) {
    SIGIL_UNUSED(handler_args);

    return {};
}

::thaumaturgy::yield cmd_playlist(const ::sigil::platform::cmd_handler_args handler_args){
    SIGIL_UNUSED(handler_args);

    return {};
}


::thaumaturgy::yield cmd_unpause(const ::sigil::platform::cmd_handler_args handler_args){
    SIGIL_UNUSED(handler_args);

    return {};
}


::thaumaturgy::yield cmd_pause(const ::sigil::platform::cmd_handler_args handler_args){
    SIGIL_UNUSED(handler_args);

    return {};
}


::thaumaturgy::yield cmd_stop(const ::sigil::platform::cmd_handler_args handler_args){
    SIGIL_UNUSED(handler_args);

    return {};
}


::thaumaturgy::yield cmd_next(const ::sigil::platform::cmd_handler_args handler_args){
    SIGIL_UNUSED(handler_args);

    return {};
}


::thaumaturgy::yield cmd_play(const ::sigil::platform::cmd_handler_args handler_args){
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