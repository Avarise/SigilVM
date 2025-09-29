#include "sigil/debug.h"
#include <sigil/platform/app.h>
#include <sigil/network/context.h>
#include <iostream>
#include <ostream>

namespace sigil::network {
    context_t::context_t(const platform::app_descriptor_t desc) {
        sigil::dcout << "Initializing Network Context for " << desc.app_name << std::endl;
    }

    context_t::~context_t() {
        //std::cout << "Removing Network Context" << std::endl;
    }
}
