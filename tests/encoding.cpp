#include <sigil/util/memory.h>
#include <sigil/status.h>
#include <gtest/gtest.h>
#include <cstdlib>
#include <cstring>
#include <ctime>

static void fill_random(uint8_t* buf, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        buf[i] = static_cast<uint8_t>(std::rand() & 0xFF);
    }
}

TEST(Encoding, Xor1GBWithMultipleKeys) {
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

    for (size_t i = 0; i < sizeof(keys)/sizeof(keys[0]); ++i) {
        // encode pass 1
        sigil::status_t s = sigil::util::xor_encode(working, SIZE, keys[i].k, keys[i].n);
        ASSERT_TRUE(s == sigil::VM_OK);

        // encode pass 2 (should revert to original)
        s = sigil::util::xor_encode(working, SIZE, keys[i].k, keys[i].n);
        ASSERT_TRUE(s == sigil::VM_OK);

        // verify equality in blocks to avoid test runner spam
        const size_t STEP = 1 << 20; // 1 MiB chunk compare
        for (size_t off = 0; off < SIZE; off += STEP) {
            size_t rem = (off + STEP <= SIZE) ? STEP : (SIZE - off);
            int cmp = std::memcmp(original + off, working + off, rem);
            EXPECT_EQ(cmp, 0) << "Mismatch at block offset " << off << " for key index " << i;
        }
    }

    std::free(original);
    std::free(working);
}
