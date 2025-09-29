#pragma once

#include <sigil/data/journal.h>
#include <sigil/data/chain.h>
#include <sigil/data/map.h>

namespace sigil::data {

struct crawler_cfg_t {
    bool follow_symlinks;
    bool deterministic;
    uint32_t workers;
};

::thaumaturgy::yield crawler_run(
    const std::filesystem::path &root,
    const crawler_cfg_t &,
    const chain_t &,
    const mapper_t &,
    journal_t &
);

}
