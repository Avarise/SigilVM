cmake_minimum_required(VERSION 3.5)

project(SigilVM
        VERSION 1.0.0
        DESCRIPTION "SigilVM apps and library")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)


set(CMAKE_CXX_STANDARD 11)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_PROTOTYPES")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_PROTOTYPES")

# GLFW
set(GLFW_DIR ./src/glfw) # Set this to point to an up-to-date GLFW repo
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)

add_subdirectory(${GLFW_DIR} binary_dir EXCLUDE_FROM_ALL)
include_directories(${GLFW_DIR}/include)

# Dear ImGui
set(IMGUI_DIR ./src/imgui)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ..)

# Libraries
find_package(Vulkan REQUIRED)
#find_library(VULKAN_LIBRARY
  #NAMES vulkan vulkan-1)
#set(LIBRARIES "glfw;${VULKAN_LIBRARY}")
set(LIBRARIES "glfw;Vulkan::Vulkan")


# ESP Compiler
# include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# Use vulkan headers from glfw:
include_directories(${GLFW_DIR}/deps)

# GLOB Core files
file(GLOB SIGIL_CORE src/core/*.cpp)

# GLOB Station module files
file(GLOB SIGIL_STATION src/station/*.cpp)

# GLOB Visor module files
file(GLOB SIGIL_VISOR src/visor/*.cpp)

# GLOB HID module files
file(GLOB SIGIL_HID src/hid/*.cpp)

# GLOB ntt component system files
file(GLOB SIGIL_NTT src/ntt/*.cpp)

# GLOB Git submodule for glfw
file(GLOB SOURCE_GLFW src/glfw/src/*.cpp)

# GLOB Git submodule for LittleFS
file(GLOB SOURCE_LFS src/lfs/*.c)

# GLOB Git submodule for Dear ImGui
file(GLOB SOURCE_IMGUI src/imgui/*.cpp)

add_executable(sigil-editor src/apps/editor/main.cpp ${SIGIL_CORE} ${SIGIL_VISOR} ${SIGIL_HID} ${SIGIL_NTT} ${SOURCE_IMGUI} ${SOURCE_GLFW})
target_link_libraries(sigil-editor ${LIBRARIES})
target_compile_definitions(sigil-editor PUBLIC -DImTextureID=ImU64 -DSIGIL_USE_GUI)

#idf_component_register(SRCS "./src/app/esp-scanner.cpp")
