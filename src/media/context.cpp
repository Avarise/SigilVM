#include <sigil/platform/app.h>
#include <sigil/media/context.h>
#include <sigil/debug.h>
#include <iostream>
#include <ostream>

namespace sigil::media {
    context_t::context_t(const platform::app_descriptor_t desc) {
        sigil::dcout << "Initializing Media Context for " << desc.app_name << std::endl;
    }

    context_t::~context_t() {
        //std::cout << "Removing Media Context" << std::endl;
    }
}