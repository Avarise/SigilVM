#include "thaumaturgy/yield.h"
#include <sigil/platform/capabilities.h>
#include <sigil/profiling/timer.h>
#include <sigil/memory/xor.h>
#include <thaumaturgy/thaumaturgy.h>

#if defined(__AVX2__)
#include <immintrin.h>
#endif

namespace sigil::memory {

::thaumaturgy::yield xor_encode(uint8_t* data, size_t size, const uint8_t* key, size_t keylen, bool disable_simd) {
    if (!data || !key || keylen == 0)
        return ::thaumaturgy::yield().set_state(thaumaturgy::yield_state::fail);

    if (!sigil::platform::has_avx2() || disable_simd) {
        // Portable fallback: XOR using uint64_t chunks when possible, then bytes.
        size_t i = 0;
        // Build a 8-byte repeating key chunk
        uint64_t key64 = 0;
        if (keylen > 0) {
            uint8_t key8[8] = {0};
            for (size_t k = 0; k < 8; ++k) key8[k] = key[k % keylen];
            memcpy(&key64, key8, 8);
        }

        // Align to 8 bytes boundary for faster operations on many CPUs.
        // Handle unaligned head
        for (; i < size && (reinterpret_cast<uintptr_t>(data + i) & 7); ++i) {
            data[i] ^= key[i % keylen];
        }

        // Bulk 8-byte XOR
        size_t n64 = (size - i) / 8;
        uint64_t* p64 = reinterpret_cast<uint64_t*>(data + i);
        for (size_t j = 0; j < n64; ++j) {
            p64[j] ^= key64;
        }
        i += n64 * 8;

        // Tail bytes
        for (; i < size; ++i) {
            data[i] ^= key[i % keylen];
        }
    } else {
        // AVX2 path: aligned start + unrolled main loop
        const size_t stride = 32;
        size_t i = 0;

        // Prepare a 32-byte repeating key block
        alignas(32) uint8_t keyblock[stride];
        for (size_t k = 0; k < stride; ++k) {
            keyblock[k] = key[k % keylen];
        }
        const __m256i kv = _mm256_load_si256(reinterpret_cast<const __m256i*>(keyblock));

        // Scalar head until 32-byte alignment
        uintptr_t addr = reinterpret_cast<uintptr_t>(data);
        size_t misalign = addr & (stride - 1);
        if (misalign) {
            size_t head = stride - misalign;
            if (head > size) head = size;
            for (size_t h = 0; h < head; ++h) {
                data[h] ^= key[h % keylen];
            }
            i += head;
        }

        // Unrolled SIMD body (128 bytes per iteration)
        const size_t unroll = stride * 4;
        for (; i + unroll <= size; i += unroll) {
            __m256i d0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(data + i +  0));
            __m256i d1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(data + i + 32));
            __m256i d2 = _mm256_load_si256(reinterpret_cast<const __m256i*>(data + i + 64));
            __m256i d3 = _mm256_load_si256(reinterpret_cast<const __m256i*>(data + i + 96));

            d0 = _mm256_xor_si256(d0, kv);
            d1 = _mm256_xor_si256(d1, kv);
            d2 = _mm256_xor_si256(d2, kv);
            d3 = _mm256_xor_si256(d3, kv);

            _mm256_store_si256(reinterpret_cast<__m256i*>(data + i +  0), d0);
            _mm256_store_si256(reinterpret_cast<__m256i*>(data + i + 32), d1);
            _mm256_store_si256(reinterpret_cast<__m256i*>(data + i + 64), d2);
            _mm256_store_si256(reinterpret_cast<__m256i*>(data + i + 96), d3);
        }

        // Remaining full SIMD blocks
        for (; i + stride <= size; i += stride) {
            __m256i d = _mm256_load_si256(reinterpret_cast<const __m256i*>(data + i));
            d = _mm256_xor_si256(d, kv);
            _mm256_store_si256(reinterpret_cast<__m256i*>(data + i), d);
        }

        // Tail
        for (; i < size; ++i) {
            data[i] ^= key[i % keylen];
        }
    }

    return ::thaumaturgy::yield();
}

} // namespace sigil::memory
