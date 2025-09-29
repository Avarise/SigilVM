#pragma once

#include <sigil/common.h>

namespace sigil::utils {
    /**
     * @brief 
     * Encode chunk of memory using arbitrary length key.
     * If AVX2 is found, SIMD is used in the process.
     * @param data 
     * @param size 
     * @param key 
     * @param keylen 
     * @return ::sigil::yield 
     */
    ::sigil::yield xor_encode(uint8_t* data, size_t size, const uint8_t* key, size_t keylen, bool disable_simd = false);

}