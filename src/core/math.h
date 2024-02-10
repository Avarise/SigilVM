#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <math.h>
#include "memory.h"

namespace sigil {
    namespace math {
        struct v3_t {
            float x;
            float y;
            float z;

            v3_t() {
                x = 0.0f;
                y = 0.0f;
                z = 0.0f;
            }
            
            v3_t(float x, float y, float z) {
                x = x;
                y = y;
                z = z;
            }
        };

        struct quat_t {
            float r;
            float x;
            float y;
            float z;

            quat_t() {
                r = 0.0f;
                x = 0.0f;
                y = 0.0f;
                z = 0.0f;
            }

            quat_t(float r, float x, float y, float z) {
                r = r;
                x = x;
                y = y;
                z = z;
            }
        };

        union uu32_t{
            uint32_t t_uint;
            char t_byte[sizeof(uint32_t)];
        };

        union uu64_t {
            uint64_t uint;
            char bytes[sizeof(uint64_t)];
            double float_non_cast;
        };

        struct color_t {
            uint8_t red;
            uint8_t green;
            uint8_t blue;
            uint8_t alpha; // used for render transparency
        };

        union pixel_t {
            color_t color;
            uint32_t absolute;
        };

        struct transform3d {
            v3_t translation;
            quat_t rotation;
            v3_t scale;

            transform3d() {
                translation.x = 0.0f;
                translation.y = 0.0f;
                translation.z = 0.0f;
                rotation.r = 0.0f;
                rotation.x = 0.0f;
                rotation.y = 0.0f;
                rotation.z = 0.0f;
                scale.x = 1.0f;
                scale.y = 1.0f;
                scale.z = 1.0f;
            }
        };

        inline sigil::math::uu32_t get_key32_from_c_str(const char *src) {
            sigil::math::uu32_t  key;
            int i = 0;

            // Expects null terminated string
            while (src[i] != '\0') {
                uint32_t temp = HASH_MASK + src[i];
                key.t_uint += temp < 1;
                key.t_byte[i % sizeof(sigil::math::uu32_t)] ^= src[i];
                i++;
            }

            return key;
        }

        // recursive impl with no depth check, not practical
        inline uint64_t u64factorial_rec(uint64_t n) {
            if (n > 1) return n * u64factorial_rec(n - 1);
            return 1;
        }

        inline uint64_t u64factorial(uint64_t n) {
            uint64_t ans = 1;

            for (int i = 1; i <= n; i++) ans *= i;
            
            return ans;
        }

        
        inline uint64_t u64binomial(uint64_t n, uint64_t k) {
            if (k > n) return 0;
            if (k == n || k == 0) return 1;
            return u64factorial(n) / (u64factorial(n - k) * u64factorial(k));
        }

        // recursive impl, not practical, only for stressing/testing
        inline uint64_t u64binomial_rec(uint64_t n, uint64_t k) {
            if (k > n) return 0;
            if (k == n || k == 0) return 1;
            return u64binomial_rec(n - 1, k - 1) + u64binomial_rec(n - 1, k);
        }

        // recursive impl, not practical, only for stressing/testing
        inline uint32_t get_gcd(uint32_t n1, uint32_t n2) {
            if (n1 > n2) return get_gcd(n1 % n2, n2);
            if (n2 > n1) return get_gcd(n1, n1 % n2);
            return n1;
        }

        inline uint32_t get_ru32(uint32_t min, uint32_t max) {
            uint32_t num = 0;

            num = ((uint32_t)std::rand() % (max + 1)) + min;

            return num;
        }
        
        inline int32_t get_rs32(int32_t min, int32_t max) {
            int32_t num = (int32_t)std::rand();
            num %= max;
            num += min;
            return num;
        }

        inline float v3_len(v3_t vec) {
            float len = 0.0f;
            len += vec.x * vec.x; 
            len += vec.y * vec.y;
            len += vec.z * vec.z;
            return sqrtf(len);
        }
    } /* end of namspace math*/
}