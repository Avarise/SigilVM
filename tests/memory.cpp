#include <sigil/profiling/timer.h>
#include <sigil/memory/xor.h>
#include <thaumaturgy/thaumaturgy.h>
#include <gtest/gtest.h>
#include <cstdlib>
#include <cstring>
#include <ctime>

static void fill_random(uint8_t* buf, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        buf[i] = static_cast<uint8_t>(std::rand() & 0xFF);
    }
}

/**
 * This test generates 1GB of data, and 8 keys of different lengths
 * We run XOR with each key once, one key after another
 * scrambling data completely. Then, we rerun the keys again;
 * as long as each key was run twice, no matter the order,
 * we should revert the data to original.
 */
TEST(Memory, Xor1GBWithMultipleKeys) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    const size_t SIZE = 1ull << 30;   // 1 GiB

    uint8_t* original = (uint8_t*)std::malloc(SIZE);
    ASSERT_NE(original, nullptr);

    uint8_t* working  = (uint8_t*)std::malloc(SIZE);
    ASSERT_NE(working, nullptr);

    fill_random(original, SIZE);
    std::memcpy(working, original, SIZE);

    // eight test keys
    static const uint8_t key0[] = { 0x11 };
    static const uint8_t key1[] = { 0x22, 0x33 };
    static const uint8_t key2[] = { 0x44, 0x55, 0x66 };
    static const uint8_t key3[] = { 0x77, 0x88, 0x99, 0xAA };
    static const uint8_t key4[] = { 0xFF, 0x00, 0x12, 0x34, 0x56 };
    static const uint8_t key5[] = { 0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6 };
    static const uint8_t key6[] = { 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70 };
    static const uint8_t key7[] = { 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x23, 0x34, 0x45 };

    struct keyinfo_t {
        const uint8_t* k;
        size_t n;
    };

    const keyinfo_t keys[] = {
        { key0, sizeof(key0) },
        { key1, sizeof(key1) },
        { key2, sizeof(key2) },
        { key3, sizeof(key3) },
        { key4, sizeof(key4) },
        { key5, sizeof(key5) },
        { key6, sizeof(key6) },
        { key7, sizeof(key7) }
    };

    ::thaumaturgy::yield s;

    for (size_t i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
        s = sigil::memory::xor_encode(working, SIZE, keys[i].k, keys[i].n);
        ASSERT_EQ(s.is_ok(), true);
    }

    for (size_t i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
        s = sigil::memory::xor_encode(working, SIZE, keys[i].k, keys[i].n);
        ASSERT_EQ(s.is_ok(), true);
    }

    const size_t STEP = 1 << 20; // 1 MiB chunk compare

    for (size_t off = 0; off < SIZE; off += STEP) {
        size_t rem = (off + STEP <= SIZE) ? STEP : (SIZE - off);
        int cmp = std::memcmp(original + off, working + off, rem);
        EXPECT_EQ(cmp, 0) << "Mismatch at block offset " << off << std::endl;
    }

    std::free(original);
    std::free(working);
}

/**
 * This test runs the XOR encoding with SIMD is actually faster than
 * non-SIMD version. To do so, we run 512MB, 1GB, and 4GB variants
 * and compare timer results at each size.
 *
 * NOTE:
 *  - This is a *performance sanity test*, not a strict benchmark.
 *  - We do NOT assert absolute timing values.
 *  - We assert SIMD path is not slower than scalar path, with a small tolerance.
 *  - Test is skipped automatically on allocation failure.
 */
TEST(Memory, XorIsFasterWithSIMD) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    static const uint8_t key[] = {
        0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x23, 0x34, 0x45,
        0x56, 0x67, 0x78, 0x89, 0xAB, 0xCD, 0xEF, 0x9A,
        0xBC, 0xDE, 0xF0, 0x12, 0x23, 0x34, 0x45, 0x56,
        0x67, 0x78, 0x89, 0xAB, 0xCD, 0xEF, 0xDE, 0xAD,
    };

    // Define the max size once; changing this will affect allocation size
    constexpr size_t MAX_SIZE = 2ull << 30; // 2GB
    uint8_t* buf_simd   = (uint8_t*)std::malloc(MAX_SIZE);
    uint8_t* buf_scalar = (uint8_t*)std::malloc(MAX_SIZE);

    if (!buf_simd || !buf_scalar) {
        if (buf_simd)   std::free(buf_simd);
        if (buf_scalar) std::free(buf_scalar);
        GTEST_SKIP() << "Skipping test (allocation failed for max buffer)";
    }

    // Fill the full SIMD buffer once with random data
    fill_random(buf_simd, MAX_SIZE);
    // Copy to scalar buffer once
    std::memcpy(buf_scalar, buf_simd, MAX_SIZE);

    struct run_t {
        size_t size;
        const char* label;
    };

    const run_t runs[] = {
        { 512ull << 20, "512MB" },
        { 1ull   << 30, "1GB"   },
        { 2ull   << 30, "2GB"   } // smaller than MAX_SIZE but uses same buffer
    };

    sigil::util::timer_t t_simd;
    sigil::util::timer_t t_scalar;
    ::thaumaturgy::yield s;

    for (size_t r = 0; r < sizeof(runs) / sizeof(runs[0]); r++) {
        const size_t SIZE = runs[r].size;

        // SIMD-enabled
        t_simd.start();
        s = sigil::memory::xor_encode(
            buf_simd,
            SIZE,
            key,
            sizeof(key),
            /* disable_simd = */ false
        );
        t_simd.stop();
        ASSERT_EQ(s.is_ok(), true);

        // Scalar-only
        t_scalar.start();
        s = sigil::memory::xor_encode(
            buf_scalar,
            SIZE,
            key,
            sizeof(key),
            /* disable_simd = */ true
        );
        t_scalar.stop();
        ASSERT_EQ(s.is_ok(), true);

        const double simd_ms   = t_simd.elapsed_milliseconds();
        const double scalar_ms = t_scalar.elapsed_milliseconds();

        std::cout
            << "[ XOR PERF ] "
            << runs[r].label
            << " | SIMD: "   << simd_ms   << " ms"
            << " | Scalar: " << scalar_ms << " ms"
            << std::endl;

        // Allow 5% tolerance
        const double tolerance = scalar_ms * 1.05;

        EXPECT_LE(simd_ms, tolerance)
            << "SIMD path slower than scalar for "
            << runs[r].label;
    }

    std::free(buf_simd);
    std::free(buf_scalar);
}

// TEST(Memory, XorIsFasterWithSIMD) {
//     std::srand(static_cast<unsigned>(std::time(nullptr)));

//     static const uint8_t key[] = {
//         0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x23, 0x34, 0x45,
//         0x56, 0x67, 0x78, 0x89, 0xAB, 0xCD, 0xEF, 0x9A,
//         0xBC, 0xDE, 0xF0, 0x12, 0x23, 0x34, 0x45, 0x56,
//         0x67, 0x78, 0x89, 0xAB, 0xCD, 0xEF, 0xDE, 0xAD,
//     };

//     struct run_t {
//         size_t size;
//         const char* label;
//     };

//     const run_t runs[] = {
//         { 512ull << 20, "512MB" },
//         { 1ull   << 30, "1GB"   },
//         { 4ull   << 30, "4GB"   }
//     };

//     for (size_t r = 0; r < sizeof(runs) / sizeof(runs[0]); r++) {
//         const size_t SIZE = runs[r].size;

//         uint8_t* buf_simd   = (uint8_t*)std::malloc(SIZE);
//         uint8_t* buf_scalar = (uint8_t*)std::malloc(SIZE);

//         if (!buf_simd || !buf_scalar) {
//             if (buf_simd)   std::free(buf_simd);
//             if (buf_scalar) std::free(buf_scalar);
//             GTEST_SKIP() << "Skipping " << runs[r].label
//                          << " (allocation failed)";
//         }

//         fill_random(buf_simd, SIZE);
//         std::memcpy(buf_scalar, buf_simd, SIZE);

//         sigil::util::timer_t t_simd;
//         sigil::util::timer_t t_scalar;
//         ::thaumaturgy::yield s;

//         // SIMD-enabled
//         t_simd.start();
//         s = sigil::memory::xor_encode(
//             buf_simd,
//             SIZE,
//             key,
//             sizeof(key),
//             /* disable_simd = */ false
//         );
//         t_simd.stop();
//         ASSERT_EQ(s.is_ok(), true);

//         // Scalar-only
//         t_scalar.start();
//         s = sigil::memory::xor_encode(
//             buf_scalar,
//             SIZE,
//             key,
//             sizeof(key),
//             /* disable_simd = */ true
//         );
//         t_scalar.stop();
//         ASSERT_EQ(s.is_ok(), true);

//         const double simd_ms   = t_simd.elapsed_milliseconds();
//         const double scalar_ms = t_scalar.elapsed_milliseconds();

//         std::cout
//             << "[ XOR PERF ] "
//             << runs[r].label
//             << " | SIMD: "   << simd_ms   << " ms"
//             << " | Scalar: " << scalar_ms << " ms"
//             << std::endl;

//         /*
//          * Allow a small margin (5%) to avoid false negatives due to
//          * scheduling noise, cache effects, or CPU frequency scaling.
//          */
//         const double tolerance = scalar_ms * 1.05;

//         EXPECT_LE(simd_ms, tolerance)
//             << "SIMD path slower than scalar for "
//             << runs[r].label;

//         std::free(buf_simd);
//         std::free(buf_scalar);
//     }
// }
