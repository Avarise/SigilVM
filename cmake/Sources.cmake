file(GLOB SRC_PROFILING ${CMAKE_SOURCE_DIR}/src/profiling/*.cpp)
file(GLOB SRC_PLATFORM  ${CMAKE_SOURCE_DIR}/src/platform/*.cpp)
file(GLOB SRC_NETWORK   ${CMAKE_SOURCE_DIR}/src/network/*.cpp)
file(GLOB SRC_RENDER    ${CMAKE_SOURCE_DIR}/src/render/*.cpp)
file(GLOB SRC_FORMAT    ${CMAKE_SOURCE_DIR}/src/format/*.cpp)
file(GLOB SRC_MEMORY    ${CMAKE_SOURCE_DIR}/src/memory/*.cpp)
file(GLOB SRC_MEDIA     ${CMAKE_SOURCE_DIR}/src/media/*.cpp)
file(GLOB SRC_REALM     ${CMAKE_SOURCE_DIR}/src/realm/*.cpp)
file(GLOB SRC_DATA      ${CMAKE_SOURCE_DIR}/src/data/*.cpp)
file(GLOB SRC_MATH      ${CMAKE_SOURCE_DIR}/src/math/*.cpp)
file(GLOB SRC_VM        ${CMAKE_SOURCE_DIR}/src/vm/*.cpp)
file(GLOB SRC_IMGUI     ${CMAKE_SOURCE_DIR}/extern/imgui/*.cpp)
file(GLOB SRC_IMGUI_BACKENDS
    ${CMAKE_SOURCE_DIR}/extern/imgui/backends/imgui_impl_vulkan.cpp
    ${CMAKE_SOURCE_DIR}/extern/imgui/backends/imgui_impl_glfw.cpp
)

set(SIGILVM_SOURCES
    ${SRC_PROFILING}
    ${SRC_PLATFORM}
    ${SRC_NETWORK}
    ${SRC_MEMORY}
    ${SRC_FORMAT}
    ${SRC_RENDER}
    ${SRC_MEDIA}
    ${SRC_REALM}
    ${SRC_UTIL}
    ${SRC_DATA}
    ${SRC_VM}
    ${SRC_MATH}
    ${SRC_IMGUI}
    ${SRC_IMGUI_BACKENDS}
)

add_library(sigilvm STATIC ${SIGILVM_SOURCES})
target_link_libraries(sigilvm
    PRIVATE
        Vulkan::Vulkan
        Threads::Threads
        glfw
        ${SIGIL_EXTRA_LIBS}
)
set_target_properties(sigilvm PROPERTIES OUTPUT_NAME sigilvm)

add_library(sigilvm_shared SHARED ${SIGILVM_SOURCES})
target_link_libraries(sigilvm_shared
    PRIVATE
        Vulkan::Vulkan
        Threads::Threads
        glfw
        ${SIGIL_EXTRA_LIBS}
)
set_target_properties(sigilvm_shared PROPERTIES OUTPUT_NAME sigilvm)
