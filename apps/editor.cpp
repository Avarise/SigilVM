/**
 * @file editor.cpp
 * @author Avarise (https://github.com/Avarise)
 * @brief SigilVM Editor (sigilvm-extra)
 * @version 0.1
 * @date 2025-12-02
 * 
 * @copyright Copyright (c) 2025
 * 
 * Asset, text, scene editor for SigilVM Game Engine
 */

#include <sigil/vm/app_descriptor.h>
#include <sigil/network/context.h>
#include <sigil/render/context.h>
#include <sigil/media/context.h>
#include <sigil/realm/context.h>
#include <sigil/vm/context.h>
#include <sigil/status.h>
#include <iostream>


int main(const int argc, const char **argv) {
    // First, creation of app descriptor
    sigil::vm::app_descriptor_t desc = sigil::vm::mk_app_descriptor(argc, argv);

    // For Editor, we want all available contexts
    // IPC, Media and Rendering, and Realm + Network for world sim
    sigil::vm::context_t vm_ctx(desc);
    sigil::media::context_t media_ctx(desc);
    sigil::realm::context_t realm_ctx(desc);
    sigil::render::context_t render_ctx(desc);
    sigil::network::context_t network_ctx(desc);

    std::cout << "Editor not implemented yet" << std::endl;
    return 0;
}