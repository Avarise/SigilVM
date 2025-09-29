#include <sigil/data/identity.h>
#include <cstring>

namespace sigil::data {

bool identity_equal(const identity_t &a, const identity_t &b) {
    if (a.size != b.size)
        return false;

    if (a.has_full && b.has_full) {
        return std::memcmp(
            a.full.bytes,
            b.full.bytes,
            sizeof(a.full.bytes)
        ) == 0;
    }

    if (a.has_partial && b.has_partial) {
        return std::memcmp(
            a.partial.bytes,
            b.partial.bytes,
            sizeof(a.partial.bytes)
        ) == 0;
    }

    return false;
}

}
