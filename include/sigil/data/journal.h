#pragma once

#include <sigil/data/identity.h>
#include <sigil/data/action.h>
#include <thaumaturgy/thaumaturgy.h>

namespace sigil::data {

struct journal_t {
    int fd;
};

::thaumaturgy::yield journal_open(journal_t &, const std::filesystem::path &);
::thaumaturgy::yield journal_append(journal_t &, const action_t &, const identity_t &);
::thaumaturgy::yield journal_replay(journal_t &, /* out */ void *index);
::thaumaturgy::yield journal_close(journal_t &);

}
