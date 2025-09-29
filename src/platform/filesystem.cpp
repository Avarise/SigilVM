#include <sigil/platform/filesystem.h>
#include <thaumaturgy/thaumaturgy.h>
#include <sigil/profiling/timer.h>
#include <system_error>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <cstddef>
#include <cstdio>
#include <string>

namespace sigil::filesystem {

file_handler_t::file_handler_t(const std::filesystem::path path) {
    this->path = path;
    this->file_size = 0;
    this->data = nullptr;

    std::error_code ec;
    auto sz = std::filesystem::file_size(this->path, ec);
    if (ec) {
        this->file_size = (size_t)-1;
        this->data = nullptr;
        return;
    }

    file_size = static_cast<size_t>(sz);



    data = static_cast<uint8_t*>(std::malloc(file_size));

    if (!data) {
        this->file_size = (size_t)-1;
        this->data = nullptr;
        return;
    }

    FILE *fp = std::fopen(path.string().c_str(), "rb");
    if (!fp) {
        std::free(data);
        this->file_size = (size_t)-1;
        this->data = nullptr;
        return;
    }

    size_t read_size = std::fread(data, 1, file_size, fp);
    std::fclose(fp);

    if (read_size != file_size) {
        std::free(data);
        this->file_size = (size_t)-1;
        this->data = nullptr;
        return;
    }
}


file_handler_t::~file_handler_t() {
    if (this->data) {
        std::free(data);
        this->data = nullptr;
    }
}

::thaumaturgy::yield file_handler_t::save_to(std::filesystem::path path) {
    ::thaumaturgy::yield ret;

    if (!data || file_size == 0)
        return ret.set_state(thaumaturgy::yield_state::fail);
        // return VM_SKIPPED;

    FILE *fp = std::fopen(path.string().c_str(), "wb");
    if (!fp)
        return ret.set_state(thaumaturgy::yield_state::fail);
        // return VM_IO_ERROR;

    size_t written = std::fwrite(data, 1, file_size, fp);
    std::fclose(fp);

    if (written != file_size)
        return ret.set_state(thaumaturgy::yield_state::fail);
        // return VM_IO_ERROR;

    return ret;
}

bool binary_exists_in_path(const char* name) {
    const char* path = getenv("PATH");
    if (!path)
        return false;

    std::string p(path);
    size_t pos = 0;

    while (true) {
        size_t next = p.find(':', pos);
        std::string dir = p.substr(pos, next - pos);
        std::filesystem::path test = std::filesystem::path(dir) / name;
        if (std::filesystem::exists(test))
            return true;
        if (next == std::string::npos)
            break;
        pos = next + 1;
    }
    return false;
}

::thaumaturgy::yield
copy_tree(std::filesystem::path &src, std::filesystem::path &dst) {
    ::thaumaturgy::yield ret;

    ::thaumaturgy::contain(ret, [&] {
        if (!std::filesystem::exists(src))
            throw std::runtime_error("source path does not exist");

        if (std::filesystem::is_regular_file(src)) {
            std::filesystem::create_directories(dst.parent_path());
            std::filesystem::copy_file(
                src,
                dst,
                std::filesystem::copy_options::overwrite_existing
            );
            return;
        }

        std::filesystem::create_directories(dst);

        for (const auto& entry : std::filesystem::recursive_directory_iterator(src)) {
            const auto relative = std::filesystem::relative(entry.path(), src);
            const auto target   = dst / relative;

            if (entry.is_directory()) {
                std::filesystem::create_directories(target);
            } else if (entry.is_regular_file()) {
                std::filesystem::create_directories(target.parent_path());
                std::filesystem::copy_file(
                    entry.path(),
                    target,
                    std::filesystem::copy_options::overwrite_existing
                );
            }
        }
    });

    return ret;
}


bool files_are_identical(const std::filesystem::path &a, const std::filesystem::path &b){
    if (!std::filesystem::exists(a) ||
        !std::filesystem::exists(b))
        return false;

    if (!std::filesystem::is_regular_file(a) ||
        !std::filesystem::is_regular_file(b))
        return false;

    if (std::filesystem::file_size(a) !=
        std::filesystem::file_size(b))
        return false;

    std::ifstream fa(a, std::ios::binary);
    std::ifstream fb(b, std::ios::binary);

    if (!fa || !fb)
        return false;

    constexpr std::size_t block_size = 4096;
    char ba[block_size];
    char bb[block_size];

    while (fa && fb) {
        fa.read(ba, block_size);
        fb.read(bb, block_size);

        const std::streamsize ra = fa.gcount();
        const std::streamsize rb = fb.gcount();

        if (ra != rb)
            return false;

        if (std::memcmp(ba, bb, static_cast<std::size_t>(ra)) != 0)
            return false;
    }

    return true;
}



} // namespace sigil::filesystem
