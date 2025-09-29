/**
 * @file player.cpp
 * @author Sebastian Czapla (https://github.com/Avarise)
 * @brief SigilVM Media Player (sigilvm-desktop)
 * @version 0.1
 * @date 2025-12-02
 * 
 * @copyright Copyright (c) 2025
 * MPRIS-compatible media player based on FFMPEG 
 */

#include "sigil/status.h"
#include <sigil/vm/app_descriptor.h>
#include <sigil/media/context.h>
#include <sigil/util/macros.h>
#include <sigil/vm/context.h>
#include <sigil/vm/parser.h>
#include <iostream>

static struct app_context_t {
    sigil::media::context_t *media_ctx;
    sigil::vm::context_t *vm_ctx;
} app_context;

// Command handler declarations
sigil::status_t cmd_previous(const std::vector<std::string> &args);
sigil::status_t cmd_playlist(const std::vector<std::string> &args);
sigil::status_t cmd_unpause(const std::vector<std::string> &args);
sigil::status_t cmd_pause(const std::vector<std::string> &args);
sigil::status_t cmd_stop(const std::vector<std::string> &args);
sigil::status_t cmd_next(const std::vector<std::string> &args);
sigil::status_t cmd_play(const std::vector<std::string> &args);

int main(const int argc, const  char **argv) {
    // First, creation of app descriptor and command registry
    sigil::vm::app_descriptor_t desc = sigil::vm::mk_app_descriptor(argc, argv);
    sigil::vm::command_registry_t reg;

    sigil::status_t st = sigil::VM_OK;
    

    sigil::vm::register_command(reg, sigil::vm::command_t({"previous"}, false, cmd_previous));
    sigil::vm::register_command(reg, sigil::vm::command_t({"playlist"}, true, cmd_playlist));
    sigil::vm::register_command(reg, sigil::vm::command_t({"unpause"}, false, cmd_pause));
    sigil::vm::register_command(reg, sigil::vm::command_t({"pause"}, false, cmd_pause));
    sigil::vm::register_command(reg, sigil::vm::command_t({"stop"}, false, cmd_stop));
    sigil::vm::register_command(reg, sigil::vm::command_t({"next"}, false, cmd_next));
    sigil::vm::register_command(reg, sigil::vm::command_t({"play"}, true, cmd_play));

    // Now, all the contexts + storing a pointer in global struct
    sigil::vm::context_t vm_ctx(desc);
    app_context.vm_ctx = &vm_ctx;

    sigil::media::context_t media_ctx(desc);
    app_context.media_ctx = &media_ctx;

    std::cout << "Media player not implemented yet" << std::endl;
    return 0;
}


sigil::status_t cmd_previous(const std::vector<std::string> &args) {
    
    
    return sigil::VM_OK;
}

sigil::status_t cmd_playlist(const std::vector<std::string> &args){
    
    
    return sigil::VM_OK;
}


sigil::status_t cmd_unpause(const std::vector<std::string> &args){
    
    
    return sigil::VM_OK;
}


sigil::status_t cmd_pause(const std::vector<std::string> &args){
    
    
    return sigil::VM_OK;
}


sigil::status_t cmd_stop(const std::vector<std::string> &args){
    
    
    return sigil::VM_OK;
}


sigil::status_t cmd_next(const std::vector<std::string> &args){
    
    
    return sigil::VM_OK;
}


sigil::status_t cmd_play(const std::vector<std::string> &args){
    
    
    return sigil::VM_OK;
}
