/**
 * @file player.cpp
 * @author Avarise (https://github.com/Avarise)
 * @brief SigilVM Media Player (sigilvm-desktop)
 * @version 0.1
 * @date 2025-12-02
 * 
 * @copyright Copyright (c) 2025
 * MPRIS-compatible media player based on FFMPEG 
 */

#include <sigil/vm/app_descriptor.h>
#include <sigil/util/macros.h>
#include <iostream>

int main(const int argc, const  char **argv) {
    sigil::vm::app_descriptor_t desc = sigil::vm::mk_app_descriptor(argc, argv);
    SIGIL_UNUSED(desc);

    std::cout << "Media player not implemented yet" << std::endl;
    return 0;
}