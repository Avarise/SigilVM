#include <thaumaturgy/thaumaturgy.h>
#include <sigil/math/xxh128.h>
#include <sigil/data/dedup.h>

#include <unordered_map>
#include <vector>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <thread>
#include <atomic>


namespace fs = std::filesystem;

namespace sigil::data {

struct move_action {
    fs::path from;
    fs::path to;
};

struct hash_result {
    fs::path path;
    std::array<std::uint8_t,16> hash;
    ::thaumaturgy::yield result;
};

// ---- helpers ---------------------------------------------------------------

static std::string hash_to_hex(const std::array<std::uint8_t,16>& h) {
    static constexpr char lut[] = "0123456789abcdef";
    std::string out;
    out.reserve(32);
    for (std::uint8_t b : h) {
        out.push_back(lut[b >> 4]);
        out.push_back(lut[b & 0x0F]);
    }
    return out;
}

static fs::path cache_file_path() {
    const char* home = std::getenv("HOME");
    fs::path base = home ? fs::path(home) : fs::temp_directory_path();
    base /= ".cache/sigilvm/dedup";

    const auto now = std::chrono::system_clock::now();
    const auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&t, &tm);

    std::ostringstream name;
    name << "run-"
         << std::put_time(&tm, "%Y%m%d-%H%M%S")
         << ".txt";

    return base / name.str();
}

::thaumaturgy::yield dedup(
    const fs::path& src,
    const fs::path& dst,
    bool dry_run
) noexcept {
    ::thaumaturgy::yield ret;

    if (src.empty() || dst.empty())
        return ret.set_state(::thaumaturgy::yield_state::fail)
                  .set_code(1);

    if (!fs::exists(src) || !fs::is_directory(src))
        return ret.set_state(::thaumaturgy::yield_state::fail)
                  .set_code(2);

    ::thaumaturgy::contain(ret, [&] {
        if (!fs::exists(dst))
            fs::create_directories(dst);
    });
    if (!ret.is_ok())
        return ret;

    // collect files
    std::vector<fs::path> files;
    for (fs::recursive_directory_iterator it(src), end; it != end; ++it) {
        if (it->is_regular_file())
            files.push_back(it->path());
    }

    if (files.empty())
        return ret;

    // parallel hashing
    const unsigned workers =
        std::max(1u, std::thread::hardware_concurrency());

    std::vector<hash_result> results(files.size());
    std::atomic<std::size_t> index{0};

    auto worker = [&]() {
        while (true) {
            std::size_t i = index.fetch_add(1, std::memory_order_relaxed);
            if (i >= files.size())
                break;

            results[i].path = files[i];

            sigil::crypto::xxh128_payload_t hp;
            hp.path = files[i];

            results[i].result = sigil::crypto::xxh128_hash(hp);
            if (results[i].result.is_ok())
                results[i].hash = hp.output;
        }
    };

    std::vector<std::thread> pool;
    pool.reserve(workers);
    for (unsigned i = 0; i < workers; ++i)
        pool.emplace_back(worker);

    for (auto& t : pool)
        t.join();

    // ---- propagate hash failures -------------------------------------------
    for (const auto& r : results) {
        if (!r.result.is_ok())
            return ret |= r.result;
    }

    // ---- phase 2: plan assembly (single-threaded) ---------------------------
    std::unordered_map<std::string, fs::path> seen_hashes;
    std::vector<move_action> actions;

    for (const auto& r : results) {
        const std::string hash_hex = hash_to_hex(r.hash);

        auto [it_hash, inserted] =
            seen_hashes.emplace(hash_hex, r.path);

        if (!inserted)
            continue;

        fs::path rel = fs::relative(r.path, src);
        fs::path target = dst / rel;

        if (fs::exists(target)) {
            fs::path parent = target.parent_path();
            target = parent / (hash_hex + "-" + target.filename().string());
        }

        actions.push_back({ r.path, target });
    }

    // ---- dry run ------------------------------------------------------------
    if (dry_run) {
        fs::path out_file = cache_file_path();

        ::thaumaturgy::contain(ret, [&] {
            fs::create_directories(out_file.parent_path());
            std::ofstream out(out_file);
            if (!out)
                throw std::runtime_error("cannot open dry-run file");

            for (const auto& a : actions)
                out << a.from << " -> " << a.to << '\n';
        });

        if (!ret.is_ok())
            return ret;

        std::cout << "[dedup] Dry run plan written to:\n  "
                  << out_file << std::endl;

        return ret;
    }

    // ---- phase 3: execute ---------------------------------------------------
    for (const auto& a : actions) {
        ::thaumaturgy::contain(ret, [&] {
            fs::create_directories(a.to.parent_path());
            fs::rename(a.from, a.to);
        });

        if (!ret.is_ok())
            return ret;
    }

    return ret;
}

} // namespace sigil::tools
