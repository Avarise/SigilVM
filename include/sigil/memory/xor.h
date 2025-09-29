#pragma once
#include <thaumaturgy/thaumaturgy.h>
#include <cstring>

namespace sigil::memory {

/**
 * @brief 
 * Encode chunk of memory using arbitrary length key.
 * If AVX2 is found, SIMD is used in the process.
 * @param data 
 * @param size 
 * @param key 
 * @param keylen 
 * @return ::thaumaturgy::yield 
 */
::thaumaturgy::yield xor_encode(uint8_t* data, size_t size, const uint8_t* key, size_t keylen, bool disable_simd = false);


} // namespace sigil::memory

