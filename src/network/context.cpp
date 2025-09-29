#include <sigil/vm/app_descriptor.h>
#include <sigil/network/context.h>
#include <iostream>
#include <ostream>

namespace sigil::network {
    context_t::context_t(const vm::app_descriptor_t desc) {
        if (desc.debug) {
            std::cout << "Initializing Network Context for " << desc.app_name << std::endl;
        }
    }

    context_t::~context_t() {
        //std::cout << "Removing Network Context" << std::endl;
    }
}