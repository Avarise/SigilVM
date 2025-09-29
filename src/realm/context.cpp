#include "sigil/debug.h"
#include <sigil/platform/app.h>
#include <sigil/realm/context.h>
#include <iostream>
#include <ostream>

namespace sigil::realm {
    context_t::context_t(const platform::app_descriptor_t desc) {
        sigil::dcout << "Initializing Realm Context for " << desc.app_name << std::endl;
    }

    context_t::~context_t() {
        //std::cout << "Removing Realm Context" << std::endl;
    }
}