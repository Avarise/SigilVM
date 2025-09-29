#pragma once
#include <cstdint>

namespace sigil::util {

    /**
     * @brief
     * Generate FNV hash based on directory and the contents
     * Uses AVX2 and multithreading to maximize speed of processing.
     * @param path
     * @return uint64_t
     */
    uint64_t hash_entire_dir(const char *path);
}
