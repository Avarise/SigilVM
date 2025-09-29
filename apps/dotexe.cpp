/**
 * @file dotexe.cpp
 * @author Sebastian Czapla (https://github.com/Avarise)
 * @brief SigilVM WINE/Proton Manager (sigilvm-extra)
 * @version 0.1
 * @date 2025-12-02
 *
 * @copyright Copyright (c) 2026
 *
 * Prefix + Runner manager for running Windows Applications
 */

#include <thaumaturgy/thaumaturgy.h>
#include <sigil/platform/compat.h>
#include <sigil/macros.h>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>

static size_t prompt_choice(size_t max) {
    uint32_t v = 0;
    printf("> ");
    fflush(stdout);

    if (scanf("%u", &v) != 1)
        return -1;

    if (v < 1 || (size_t)v > max)
        return -1;

    return v - 1;
}

int main(const int argc, const char **argv) {
    std::vector<sigil::platform::compat_profile_t> profiles = {};
    std::vector<sigil::platform::compat_tool_t> runners = {};

    if (argc == 1) {
        sigil::platform::probe_compat_profiles(profiles);

        size_t choice;
        do {
            std::cout << "SigilVM Dotexe profiles:" << std::endl;

            for (size_t i = 0; i < profiles.size(); i++) {
                std::cout << i+1 << ". " << profiles.at(i).target.string() << std::endl;
            }

            choice = prompt_choice(profiles.size());
        } while (choice > profiles.size());

        sigil::platform::run_compat_profile(profiles.at(choice));
    }

    const char* exe_path = argv[1];
    bool force_config = false;

    if (argc > 2 && std::strcmp(argv[2], "--configure") == 0)
         force_config = true;

    // Always probe new compat tools
    ::thaumaturgy::yield s = sigil::platform::probe_compat_tools(runners);
    // if (s != auto{}) {
    //     std::cout << "Error when probing for tools: " << s << std::endl;
    // }

    SIGIL_UNUSED(s);
    SIGIL_UNUSED(exe_path);
    SIGIL_UNUSED(force_config);

    return 0;
}
