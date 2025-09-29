#pragma once
#include <thaumaturgy/thaumaturgy.h>
#include <filesystem>
#include <cstdint>
#include <cstdint>
#include <string>


namespace sigil::math {

struct sha256_t {
    uint8_t bytes[32];
    std::string string() const;
};




// enum sha256_mode_t : uint32_t {
//     SHA256_FULL,        // full file
//     SHA256_PARTIAL      // front + back windows
// };

// struct sha256_cfg_t {
//     sha256_mode_t mode;
//     uint64_t window_bytes;   // used only for PARTIAL
// };

// /* core streaming API */
// struct sha256_ctx_t {
//     uint32_t state[8];
//     uint64_t bitlen;
//     uint8_t  buffer[64];
//     uint32_t buffer_len;
// };

// void sha256_init(sha256_ctx_t &);
// void sha256_update(sha256_ctx_t &, const uint8_t *data, uint64_t len);
// void sha256_final(sha256_ctx_t &, sha256_t &out);

// /* file helpers */
// ::thaumaturgy::yieldsha256_file(
//     const std::filesystem::path &path,
//     sha256_t &out
// );

// ::thaumaturgy::yieldsha256_file_cfg(
//     const std::filesystem::path &path,
//     const sha256_cfg_t &cfg,
//     sha256_t &out
// );

// /* ============================
//  * FNV-1a (fast, non-crypto)
//  * ============================ */

// using fnv64_t = uint64_t;

// fnv64_t fnv1a_bytes(const uint8_t *data, uint64_t len);

// ::thaumaturgy::yieldfnv1a_file(
//     const std::filesystem::path &path,
//     fnv64_t &out
// );


// /**
//  * 
//  */
// ::thaumaturgy::yieldhash_by_path(
//     const std::filesystem::path &path,
//     fnv64_t &out
// );


// /**
//  * @brief
//  * Generate FNV hash based on directory and the contents
//  * Uses AVX2 and multithreading to maximize speed of processing.
//  * @param path
//  * @return uint64_t
//  * DEPRECATED
//  */
// uint64_t hash_entire_dir(const char *path);

} // namespace sigil::math
