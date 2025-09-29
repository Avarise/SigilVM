#include <sigil/vm/app_descriptor.h>
#include <sigil/realm/context.h>
#include <iostream>
#include <ostream>

namespace sigil::realm {
    context_t::context_t(const vm::app_descriptor_t desc) {
        if (desc.debug) {
            std::cout << "Initializing Realm Context for " << desc.app_name << std::endl;
        }
    }

    context_t::~context_t() {
        //std::cout << "Removing Realm Context" << std::endl;
    }
}