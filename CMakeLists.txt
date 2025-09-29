cmake_minimum_required(VERSION 3.16)

project(SigilVM
    VERSION 1.0.0
    DESCRIPTION "SigilVM Ecosystem — Media Engine for Linux"
    LANGUAGES C CXX
)

# ==== Core Modules ====
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(Paths)          # output dirs, .cache
include(Options)        # knobs: testing, wayland, linters, shaders
include(BuildFlags)     # -Wall -Wextra -march=native etc.
include(Dependencies)   # Vulkan, glfw, X11, threads
include(Sources)        # file(GLOB ...) and set(SIGILVM_SOURCES)
include(Applications)   # sigilvm-editor, tools, xorit
include(Testing)        # BUILD_TESTING + test targets
include(ClangTidy)      
include(Valgrind)

# ==== Final Summary ====
message(STATUS "SigilVM Ready. Build dir: ${CMAKE_BINARY_DIR}")
