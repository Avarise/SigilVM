#include <sigil/platform/capabilities.h>
#include <sigil/math/hash.h>
#include <system_error>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>

#ifdef __AVX2__
#include <immintrin.h>
#endif

namespace fs = std::filesystem;

namespace sigil::math {

static constexpr uint64_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
static constexpr uint64_t FNV_PRIME = 1099511628211ULL;

static inline uint64_t mix_u64(uint64_t acc, uint64_t v) {
    // simple FNV-like mix
    acc ^= v;
    acc *= FNV_PRIME;
    return acc;
}

static uint64_t fnv1a_chunk(const uint8_t* data, size_t len, uint64_t acc) {
    uint64_t h = acc;
    for (size_t i = 0; i < len; ++i) {
        h ^= static_cast<uint64_t>(data[i]);
        h *= FNV_PRIME;
    }
    return h;
}

#ifdef __AVX2__
// Reduce a 256-bit accumulator (4 x 64) into a single 64-bit value deterministically
static inline uint64_t reduce256_to_u64(__m256i acc256, uint64_t seed) {
    // Extract 4 lanes and mix into 64-bit
    alignas(32) uint64_t lanes[4];
    _mm256_store_si256(reinterpret_cast<__m256i*>(lanes), acc256);
    uint64_t r = seed;
    r = mix_u64(r, lanes[0]);
    r = mix_u64(r, lanes[1]);
    r = mix_u64(r, lanes[2]);
    r = mix_u64(r, lanes[3]);
    return r;
}
#endif

static uint64_t hash_file_scalar(const fs::path &p) {
    std::ifstream ifs(p, std::ios::binary);
    if (!ifs.is_open()) return 0ULL;

    const size_t BUF_SZ = 64 * 1024; // 64KB
    std::vector<uint8_t> buf;
    buf.resize(BUF_SZ);

    uint64_t h = FNV_OFFSET_BASIS;
    while (ifs) {
        ifs.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(BUF_SZ));
        std::streamsize got = ifs.gcount();
        if (got <= 0) break;
        h = fnv1a_chunk(buf.data(), static_cast<size_t>(got), h);
    }
    return h;
}

#ifdef __AVX2__
static uint64_t hash_file_avx2(const fs::path &p) {
    std::ifstream ifs(p, std::ios::binary);
    if (!ifs.is_open()) return 0ULL;

    const size_t BUF_SZ = 64 * 1024; // 64KB
    std::vector<uint8_t> buf;
    buf.resize(BUF_SZ);

    // 256-bit accumulator (4x64)
    __m256i acc256 = _mm256_setzero_si256();
    uint64_t mixed = FNV_OFFSET_BASIS;

    while (ifs) {
        ifs.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(BUF_SZ));
        std::streamsize got = ifs.gcount();
        if (got <= 0) break;

        const uint8_t* data = buf.data();
        size_t remaining = static_cast<size_t>(got);

        // process 32-byte blocks with AVX2 XOR accumulation
        while (remaining >= 32) {
            __m256i m = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data));
            acc256 = _mm256_xor_si256(acc256, m);
            data += 32;
            remaining -= 32;
        }

        // reduce acc256 into mixed to avoid unbounded growth
        mixed = mix_u64(mixed, reduce256_to_u64(acc256, 0));
        acc256 = _mm256_setzero_si256();

        // handle remainder with scalar FNV
        if (remaining > 0) {
            mixed = fnv1a_chunk(data, remaining, mixed);
        }
    }

    // final mix
    return mix_u64(mixed, 0x9E3779B97F4A7C15ULL);
}
#endif

static uint64_t hash_file(const fs::path &p) {
    if (platform::has_avx2())
        return hash_file_avx2(p);
    else
        return hash_file_scalar(p);
}

// Helper: read filesize safely
static uint64_t safe_filesize(const fs::path &p) {
    std::error_code ec;
    auto sz = fs::file_size(p, ec);
    if (ec) return 0ULL;
    return static_cast<uint64_t>(sz);
}

// Primary exported function
uint64_t hash_entire_dir(const char *cpath) {
    if (!cpath) return 0ULL;
    std::string root = cpath;
    std::error_code ec;

    // Gather regular files recursively
    std::vector<fs::path> files;
    try {
        if (!fs::exists(root, ec) || !fs::is_directory(root, ec)) {
            return 0ULL;
        }

        for (auto it = fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied, ec);
             it != fs::recursive_directory_iterator(); it.increment(ec)) {

            if (ec) {
                // skip entries that caused errors
                ec.clear();
                continue;
            }

            const fs::directory_entry &ent = *it;
            if (!ent.is_regular_file(ec)) continue;
            files.push_back(ent.path());
        }
    } catch (...) {
        // on any exception, return 0
        return 0ULL;
    }

    if (files.empty()) return 0ULL;

    // sort paths lexicographically to enforce deterministic order
    std::sort(files.begin(), files.end(), [](const fs::path &a, const fs::path &b) {
        auto sa = a.generic_u8string();
        auto sb = b.generic_u8string();
        return sa < sb;
    });

    // Prepare results vector
    std::vector<uint64_t> per_file_hash(files.size(), 0);

    // Launch worker threads to hash files in parallel.
    unsigned int hw = std::thread::hardware_concurrency();
    if (hw == 0) hw = 2;
    unsigned int num_threads = std::min<unsigned int>(hw, static_cast<unsigned int>(files.size()));

    std::atomic<size_t> next_idx{0};
    std::vector<std::thread> workers;
    workers.reserve(num_threads);

    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&]() {
            for (;;) {
                size_t idx = next_idx.fetch_add(1, std::memory_order_relaxed);
                if (idx >= files.size()) break;
                const fs::path &p = files[idx];

                uint64_t fh = 0ULL;
                try {
                    fh = hash_file(p);
                } catch (...) {
                    fh = 0ULL;
                }
                per_file_hash[idx] = fh;
            }
        });
    }

    for (auto &th : workers) th.join();

    // Combine file hashes in deterministic sorted order, mixing path and size metadata
    uint64_t final_hash = FNV_OFFSET_BASIS;
    for (size_t i = 0; i < files.size(); ++i) {
        const auto &p = files[i];
        uint64_t fh = per_file_hash[i];
        auto pathstr = p.generic_u8string();

        // mix path bytes
        final_hash = fnv1a_chunk(reinterpret_cast<const uint8_t*>(pathstr.data()), pathstr.size(), final_hash);

        // mix size
        uint64_t fsize = safe_filesize(p);
        final_hash = mix_u64(final_hash, fsize);

        // mix file content hash
        final_hash = mix_u64(final_hash, fh);

        // extra churn
        final_hash ^= (uint64_t)(0xA5A5A5A5A5A5A5A5ULL ^ (i * 0x9E3779B97F4A7C15ULL));
        final_hash *= 6364136223846793005ULL;
    }

    // Final avalanche
    final_hash ^= (final_hash >> 33);
    final_hash *= 0xff51afd7ed558ccdULL;
    final_hash ^= (final_hash >> 33);
    final_hash *= 0xc4ceb9fe1a85ec53ULL;
    final_hash ^= (final_hash >> 33);

    return final_hash;
}

} // namespace sigil::util
