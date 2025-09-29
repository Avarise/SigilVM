/**
 * @file xorit.cpp
 * @author Avarise (https://github.com/Avarise)
 * @brief SigilVM Xorit (sigilvm-extra)
 * @version 1.0
 * @date 2025-12-02
 * 
 * @copyright Copyright (c) 2025
 * 
 * Asset obfuscation utility for SigilVM
 */

#include <sigil/util/memory.h>
#include <sigil/util/fs.h>
#include <sigil/status.h>
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

    sigil::util::file_handler_t key_file(args.keyfile);
    sigil::util::file_handler_t source(args.infile);

    sigil::status_t st = sigil::util::xor_encode(source.data, source.file_size, key_file.data, key_file.file_size);

    if (st != sigil::VM_OK) {
        std::cout << "[ERROR] Encoding failed" << std::endl;
        return 3;
    }

    source.save_to(args.outfile);
    

    return 0;
}