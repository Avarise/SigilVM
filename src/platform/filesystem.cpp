#include <sigil/platform/filesystem.h>
#include <sigil/profiling/timer.h>
#include <sigil/status.h>
#include <system_error>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <cstddef>
#include <cstdio>
#include <string>

namespace sigil::filesystem {

file_handler_t::file_handler_t(const std::filesystem::path path) {
    this->path = path;
    this->file_size = 0;
    this->data = nullptr;
    this->status = VM_OK;

    std::error_code ec;
    auto sz = std::filesystem::file_size(this->path, ec);
    if (ec) {
        status = VM_IO_ERROR;
        return;
    }

    file_size = static_cast<size_t>(sz);



    data = static_cast<uint8_t*>(std::malloc(file_size));

    if (!data) {
        status = VM_OUT_OF_MEMORY;
        file_size = 0;
        return;
    }

    FILE *fp = std::fopen(path.string().c_str(), "rb");
    if (!fp) {
        status = VM_IO_ERROR;
        std::free(data);
        data = nullptr;
        file_size = 0;
        return;
    }

    size_t read_size = std::fread(data, 1, file_size, fp);
    std::fclose(fp);

    if (read_size != file_size) {
        status = VM_IO_ERROR;
        std::free(data);
        data = nullptr;
        file_size = 0;
        return;
    }
}


file_handler_t::~file_handler_t() {
    if (this->data) {
        std::free(data);
        this->data = nullptr;
    }
}

status_t file_handler_t::save_to(std::filesystem::path path) {
    if (!data || file_size == 0)
        return VM_SKIPPED;

    FILE *fp = std::fopen(path.string().c_str(), "wb");
    if (!fp)
        return VM_IO_ERROR;

    size_t written = std::fwrite(data, 1, file_size, fp);
    std::fclose(fp);

    if (written != file_size)
        return VM_IO_ERROR;

    return VM_OK;
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


} // namespace sigil::filesystem
