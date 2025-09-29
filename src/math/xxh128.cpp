#include <thaumaturgy/thaumaturgy.h>
#include <sigil/math/xxh128.h>
#include <fstream>
#include <vector>

extern "C" {
#include <xxhash/xxhash.h>
}

namespace sigil::crypto {

::thaumaturgy::yield xxh128_hash(xxh128_payload_t& payload) noexcept {
    ::thaumaturgy::yield ret;

    if (payload.path.empty())
        return ret.set_state(::thaumaturgy::yield_state::fail)
                  .set_code(1);

    std::ifstream file;
    std::uint64_t file_size = 0;

    // ---- filesystem operations (may throw) ----
    ::thaumaturgy::contain(ret, [&] {
        file.open(payload.path, std::ios::binary | std::ios::ate);
        if (!file)
            throw std::runtime_error("open failed");

        file_size = static_cast<std::uint64_t>(file.tellg());
        file.seekg(0, std::ios::beg);
    });

    if (!ret.is_ok())
        return ret;

    // ---- XXH3 streaming state ----
    XXH3_state_t* state = XXH3_createState();
    if (!state)
        return ret.set_state(::thaumaturgy::yield_state::fail)
                  .set_code(2);

    XXH3_128bits_reset(state);

    constexpr std::size_t buffer_size = 256 * 1024; // 256 KiB
    std::vector<std::uint8_t> buffer(buffer_size);

    // ---- streaming read + hash ----
    ::thaumaturgy::contain(ret, [&] {
        while (file_size > 0) {
            const std::size_t to_read =
                static_cast<std::size_t>(
                    std::min<std::uint64_t>(buffer_size, file_size));

            file.read(reinterpret_cast<char*>(buffer.data()), to_read);
            if (!file)
                throw std::runtime_error("read failed");

            XXH3_128bits_update(state, buffer.data(), to_read);
            file_size -= to_read;
        }
    });

    if (!ret.is_ok()) {
        XXH3_freeState(state);
        return ret;
    }

    // ---- finalize ----
    const XXH128_hash_t h = XXH3_128bits_digest(state);
    XXH3_freeState(state);

    // store little-endian, stable byte order
    std::uint64_t lo = h.low64;
    std::uint64_t hi = h.high64;

    for (int i = 0; i < 8; ++i) {
        payload.output[i]     = static_cast<std::uint8_t>(lo >> (i * 8));
        payload.output[i + 8] = static_cast<std::uint8_t>(hi >> (i * 8));
    }

    return ret;
}

} // namespace sigil::crypto
