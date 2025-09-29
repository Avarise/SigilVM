#pragma once

#include <sigil/data/classify.h>

namespace sigil::data {

struct chain_t {
    classifier_t *items;
    uint32_t      count;
};

::thaumaturgy::yield chain_run(
    const chain_t &,
    const file_info_t &,
    action_t &
);

}
