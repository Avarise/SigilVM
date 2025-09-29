#pragma once

/**
 * Process header for SigilVM, centered around 
 * sigil::platform::process_descriptor_t
 * Mandatory for use with exec wrappers.
 */
 
namespace sigil::platform {

struct process_descriptor_t {
    const int    argc;
    const char **argv;
    const char **envp;
    
    process_descriptor_t(const int argc, const char **argv, const char **envp = nullptr);
};

} // namespace sigil::platform