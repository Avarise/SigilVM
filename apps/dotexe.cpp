#include <cstdint>
#include <sigil/platform/compat.h>
#include <sigil/status.h>
#include <iostream>
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
        } while (choice < 0 || choice > profiles.size());

        sigil::platform::run_compat_profile(profiles.at(choice));
    }

    const char* exe_path = argv[1];
    bool force_config = false;

    if (argc > 2 && std::strcmp(argv[2], "--configure") == 0)
         force_config = true;

    // Always probe new compat tools
    sigil::status_t s = sigil::platform::probe_compat_tools(runners);
    if (s != sigil::VM_OK) {
        std::cout << "Error when probing for tools: " << s << std::endl;
    }



    return 0;
}
