#pragma once

#include <cstdint>
#include <vector>
#include <utility>

#include "yield.h"

namespace sigil {

struct entity final {
    std::uint64_t id         = 0;
    std::uint64_t generation = 0;

    constexpr bool operator==(const entity& o) const noexcept {
        return id == o.id && generation == o.generation;
    }

    constexpr bool operator!=(const entity& o) const noexcept {
        return !(*this == o);
    }

    constexpr explicit operator bool() const noexcept {
        return id != 0;
    }
};


struct ledger {
    // opaque storage – implementation-defined
    std::vector<entity> alive;
};

inline entity create(ledger& l) noexcept {
    entity e;
    e.id = l.alive.size() + 1;
    e.generation = 1;
    l.alive.push_back(e);
    return e;
}

inline yield retire(ledger& l, entity e) noexcept {
    for (auto& x : l.alive) {
        if (x == e) {
            x.generation++; // invalidate
            return {};
        }
    }
    return yield(trap_state::fail).set_code(1); // entity not found
}

inline bool exists(const ledger& l, entity e) noexcept {
    for (const auto& x : l.alive) {
        if (x == e) return true;
    }
    return false;
}

struct ownership_tree {
    // child -> parent
    std::vector<std::pair<entity, entity>> edges;
};

inline bool owns(
    const ownership_tree& t,
    entity parent,
    entity child
) noexcept {
    entity cur = child;
    while (cur) {
        for (const auto& [c, p] : t.edges) {
            if (c == cur) {
                if (p == parent) return true;
                cur = p;
                goto next;
            }
        }
        break;
    next:;
    }
    return false;
}

inline yield attach(
    ownership_tree& t,
    entity parent,
    entity child
) noexcept {
    if (!parent || !child || parent == child)
        return yield(trap_state::fail).set_code(2);

    if (owns(t, child, parent))
        return yield(trap_state::fail).set_code(3); // cycle

    t.edges.emplace_back(child, parent);
    return {};
}

inline yield detach(
    ownership_tree& t,
    entity child
) noexcept {
    for (auto it = t.edges.begin(); it != t.edges.end(); ++it) {
        if (it->first == child) {
            t.edges.erase(it);
            return {};
        }
    }
    return yield(trap_state::partial).set_code(4); // no parent
}

} // namespace sigil
