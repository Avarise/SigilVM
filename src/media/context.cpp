#include <sigil/vm/app_descriptor.h>
#include <sigil/media/context.h>
#include <iostream>
#include <ostream>

namespace sigil::media {
    context_t::context_t(const vm::app_descriptor_t desc) {
        if (desc.debug) {
            std::cout << "Initializing Media Context for " << desc.app_name << std::endl;
        }
    }

    context_t::~context_t() {
        //std::cout << "Removing Media Context" << std::endl;
    }
}