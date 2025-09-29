cmake_minimum_required(VERSION 3.16)

project(SigilVM
    VERSION 1.0.0
    DESCRIPTION "SigilVM Ecosystem"
    LANGUAGES C CXX
)

# ==== Core Modules ====
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# -Wall -Wextra -march=native etc.
include(BuildFlags)

# Vulkan, glfw, X11, threads
include(Dependencies)   

# Enable or modify linters, testing, and other switches.
include(Options)

# Premade paths for various dirs: out, .cache, src, include/sigil (public headers)
# include/
include(Paths)

# File Globs, add_executable, target_link_libraries happen here.
# Final binaries: sigilvm-tools, sigilvm-editor, sigilvm-player, sigilvm-xorit
include(Targets)

# Testing file globs, binaries and other specifics.
# Valgrind should happen here too.
include(Testing)


# ==== Final Summary ====
message(STATUS "SigilVM Ready. Build dir: ${CMAKE_BINARY_DIR}")
