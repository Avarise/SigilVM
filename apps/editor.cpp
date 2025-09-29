/**
 * @file editor.cpp
 * @author Sebastian Czapla (https://github.com/Avarise)
 * @brief SigilVM Editor (sigilvm-extra)
 * @version 0.1
 * @date 2025-12-02
 *
 * @copyright Copyright (c) 2026
 *
 * Asset, text, scene editor for SigilVM Game Engine
 */

#include <thaumaturgy/thaumaturgy.h>
#include <sigil/network/context.h>
#include <sigil/render/context.h>
#include <sigil/media/context.h>
#include <sigil/realm/context.h>
#include <sigil/platform/app.h>
#include <iostream>


int main(const int argc, const char **argv, const char **envp) {
    // First, creation of app descriptor
    sigil::platform::process_descriptor_t proc(argc, argv, envp);
    sigil::platform::app_descriptor_t app(proc);

    // For Editor, we want all available contexts
    // IPC, Media and Rendering, and Realm + Network for world sim
    sigil::media::context_t media_ctx(app);
    sigil::realm::context_t realm_ctx(app);
    sigil::render::context_t render_ctx(app);
    sigil::network::context_t network_ctx(app);

    std::cout << "Editor not implemented yet" << std::endl;
    return 0;
}
