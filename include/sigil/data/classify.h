#pragma once

#include <sigil/data/fileinfo.h>
#include <sigil/data/action.h>
#include <thaumaturgy/thaumaturgy.h>

namespace sigil::data {

using classify_fn_t = ::thaumaturgy::yield(*)(
    const file_info_t &,
    action_t &,
    void *user
);

struct classifier_t {
    classify_fn_t fn;
    void         *user;
};

}
