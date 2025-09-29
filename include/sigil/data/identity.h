#pragma once

#include <sigil/math/hash.h>

namespace sigil::data {

struct identity_t {
    uint64_t size;

    bool has_partial;
    bool has_full;

    math::sha256_t partial;
    math::sha256_t full;
};

bool identity_equal(const identity_t &, const identity_t &);

}
