# Build / packaging work dirs, ignored by git
set(SIGIL_DIR_OUT ${CMAKE_SOURCE_DIR}/out)
set(SIGIL_DIR_BUILD ${CMAKE_SOURCE_DIR}/.cache/build)

# ensure dirs exist
file(MAKE_DIRECTORY ${SIGIL_DIR_BUILD})
file(MAKE_DIRECTORY ${SIGIL_DIR_OUT})
file(MAKE_DIRECTORY ${SIGIL_DIR_OUT}/bin)
file(MAKE_DIRECTORY ${SIGIL_DIR_OUT}/lib)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${SIGIL_DIR_OUT}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SIGIL_DIR_OUT}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SIGIL_DIR_OUT}/bin)

# CMake's own files (cache, intermediates)
set(CMAKE_CACHEFILE_DIR ${SIGIL_DIR_BUILD})
set(CMAKE_BINARY_DIR ${SIGIL_DIR_BUILD})

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/imgui
    ${CMAKE_SOURCE_DIR}/include/imgui/backends
)

add_subdirectory(include/xxhash/build/cmake xxhash_build EXCLUDE_FROM_ALL)