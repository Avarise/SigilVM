#pragma once
/*
    Spawner engine provides functionality to mass create entities
*/

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <chrono>

#include "ntt.h"
#include "general-components.h"
#include "graphic-components.h"
#include "game-components.h"
#include "../core/memory.h"

namespace sigil {
    namespace spawner {
        int spawn_random(ntt::scene_t *target, int num_entities);
    }
}