/**
 * @file xorit.cpp
 * @author Sebastian Czapla (https://github.com/Avarise)
 * @brief SigilVM Xorit (sigilvm-extra)
 * @version 1.0
 * @date 2025-12-02
 *
 * @copyright Copyright (c) 2026
 *
 * Asset obfuscation utility for SigilVM
 */

#include <sigil/platform/filesystem.h>
#include <thaumaturgy/thaumaturgy.h>
#include <sigil/profiling/timer.h>
#include <sigil/format/strings.h>
#include <sigil/memory/xor.h>
#include <filesystem>
#include <iostream>

struct cmd_args_t {
    std::filesystem::path keyfile;
    std::filesystem::path infile;
    std::filesystem::path outfile;
    bool valid() const { return !keyfile.empty() && !infile.empty() && !outfile.empty(); }
};

static cmd_args_t parse_args(const int argc, const char** argv) {
    cmd_args_t a;
    if (argc < 4) return a;
    a.keyfile = argv[1];
    a.infile = argv[2];
    a.outfile = argv[3];
    return a;
}

int main(const int argc, const char **argv) {
    cmd_args_t args = parse_args(argc, argv);

    if (!args.valid()) {
        std::cerr << "[ERROR] Usage: sigilvm-xorit <keyfile> <infile> <outfile>\n";
        return 2;
    }

    sigil::filesystem::file_handler_t key_file(args.keyfile);

    sigil::util::timer_t tm;
    tm.start();

    sigil::filesystem::file_handler_t source(args.infile);

    tm.stop();
    std::cout << "[INFO] " << source.path.string() << " [" 
              << sigil::format::bytes_pretty(source.file_size) 
              << "] loaded in " << tm.elapsed_milliseconds() 
              << " ms" << std::endl;

    tm.start();
    ::thaumaturgy::yield st = sigil::memory::xor_encode(source.data, source.file_size, key_file.data, key_file.file_size);

    if (st.is_failure()) {
        tm.stop();
        std::cout << "[ERROR] Encoding failed after " << tm.elapsed_milliseconds() << "ms" << std::endl;
        return 3;
    }

    std::cout << "[INFO] Encoding done in " << tm.elapsed_milliseconds() << "ms" << std::endl;

    tm.start();
    source.save_to(args.outfile);
    tm.stop();
    
    std::cout << "[INFO] " << args.outfile.string() << " [" 
              << sigil::format::bytes_pretty(source.file_size)
              << "] saved in " << tm.elapsed_milliseconds() 
              << " ms" << std::endl;

    return 0;
}
