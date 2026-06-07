project(SigilVM
    VERSION 1.0.0
    DESCRIPTION "Sigil Virtual Machine"
    LANGUAGES C CXX
)

# ============================================================
# Load Core Build Configuration
# ============================================================

include(${CMAKE_SOURCE_DIR}/cmake/version.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/options.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/paths.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/compiler.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/dependencies.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/targets.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/apps.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/testing.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/install.cmake)

# ============================================================
# Load Source Tree
# ============================================================

include(${CMAKE_SOURCE_DIR}/src/CMakeLists.txt)

# ============================================================
# Load Applications
# ============================================================

include(${CMAKE_SOURCE_DIR}/src/apps/CMakeLists.txt)

# ============================================================
# Tests
# ============================================================

if(SIGILVM_BUILD_TESTS)
    include(${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt)
endif()

# ============================================================
# Summary
# ============================================================

include(${CMAKE_SOURCE_DIR}/cmake/summary.cmake)