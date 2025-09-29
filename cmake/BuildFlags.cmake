set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -march=native -mavx2")

# Do not build xxhash cmd util
set(XXHASH_BUILD_XXHSUM OFF) 

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(SIGIL_DEBUG)
endif()
