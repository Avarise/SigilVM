#include "sigil/debug.h"
#include <sigil/platform/app.h>
#include <sigil/vm/context.h>
#include <iostream>
#include <ostream>

namespace sigil::vm {
    context_t::context_t(const platform::app_descriptor_t desc) {
        sigil::dcout << "Initializing VM Context for " << desc.app_name << std::endl;
    }

    context_t::~context_t() {
        //std::cout << "Removing VM Context" << std::endl;
    }
}
