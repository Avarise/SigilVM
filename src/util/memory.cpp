#include <sigil/util/memory.h>
#include <sigil/util/timer.h>
#include <sigil/platform.h>
#include <sigil/status.h>
#include <iostream>

#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
#endif

namespace sigil::util {


sigil::status_t xor_encode(uint8_t* data, size_t size, const uint8_t* key, size_t keylen) {
    if (!data || !key || keylen == 0) return sigil::VM_ARG_INVALID;

    sigil::util::timer_t t;

    t.start();

    if (sigil::has_avx2_extensions()) {
        // AVX2 path: process 32 bytes per iteration using unaligned loads
        const size_t stride = 32;
        size_t i = 0;

        // Prepare a 32-byte key block that repeats the key
        uint8_t keyblock[stride];
        for (size_t k = 0; k < stride; ++k) {
            keyblock[k] = key[k % keylen];
        }
        __m256i kv = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(keyblock));

        // Main loop: XOR 32 bytes at a time
        for (; i + stride <= size; i += stride) {
            __m256i d = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data + i));
            __m256i r = _mm256_xor_si256(d, kv);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(data + i), r);
            // rotate key vector for next block if keylen not a divisor of 32
            // but since we've built a repeating keyblock it's already aligned to repeating pattern
        }

        // tail
        size_t remaining = size - i;
        if (remaining) {
            for (size_t t = 0; t < remaining; ++t) {
                data[i + t] ^= key[(i + t) % keylen];
            }
        }

        t.stop();
        std::cout << "[INFO] XOR encoded in " << t.elapsed_milliseconds() << "ms" << std::endl;
        return sigil::VM_OK;
    } 
    
    else {
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

        t.stop();
        std::cout << "[INFO] XOR encoded in " << t.elapsed_milliseconds() << "ms" << std::endl;
        return sigil::VM_OK;
    }
}

}