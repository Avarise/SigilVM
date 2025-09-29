#include <sigil/vm/app_descriptor.h>
#include <sigil/vm/context.h>
#include <iostream>
#include <ostream>

namespace sigil::vm {
    context_t::context_t(const app_descriptor_t desc) {
        if (desc.debug) {
            std::cout << "Initializing VM Context for " << desc.app_name << std::endl;
        }
    }

    context_t::~context_t() {
        //std::cout << "Removing VM Context" << std::endl;
    }
}