if(NOT BUILD_EXAMPLES)
    return()
endif()

file(GLOB SRC_IMGUI     ${CMAKE_SOURCE_DIR}/include/imgui/*.cpp)
file(GLOB SRC_PLATFORM  ${CMAKE_SOURCE_DIR}/src/platform/*.cpp)
file(GLOB SRC_NETWORK   ${CMAKE_SOURCE_DIR}/src/network/*.cpp)
file(GLOB SRC_RENDER    ${CMAKE_SOURCE_DIR}/src/render/*.cpp)
file(GLOB SRC_UTILS     ${CMAKE_SOURCE_DIR}/src/utils/*.cpp)
file(GLOB SRC_MEDIA     ${CMAKE_SOURCE_DIR}/src/media/*.cpp)
file(GLOB SRC_GAME      ${CMAKE_SOURCE_DIR}/src/game/*.cpp)
file(GLOB SRC_MATH      ${CMAKE_SOURCE_DIR}/src/math/*.cpp)
file(GLOB SRC_GUI      ${CMAKE_SOURCE_DIR}/src/gui/*.cpp)
file(GLOB SRC_VM        ${CMAKE_SOURCE_DIR}/src/vm/*.cpp)

set(SIGILVM_SOURCES
    ${SRC_PLATFORM}
    ${SRC_NETWORK}
    ${SRC_RENDER}
    ${SRC_MEDIA}
    ${SRC_UTILS}
    ${SRC_DATA}
    ${SRC_MATH}
    ${SRC_GAME}
    ${SRC_GUI}
    ${SRC_VM}
    ${SRC_IMGUI}
)

add_library(sigilvm STATIC ${SIGILVM_SOURCES} ${SRC_IMGUI})
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

add_executable(sigilvm-editor ${CMAKE_SOURCE_DIR}/apps/editor.cpp ${SRC_IMGUI})
target_link_libraries(sigilvm-editor PRIVATE sigilvm)

if(ENABLE_WAYLAND)
    target_compile_definitions(sigilvm-editor PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
endif()

add_executable(sigilvm-player  ${CMAKE_SOURCE_DIR}/apps/player.cpp)
target_link_libraries(sigilvm-player PRIVATE sigilvm)

add_executable(sigilvm-tools  ${CMAKE_SOURCE_DIR}/apps/tools.cpp)
target_link_libraries(sigilvm-tools PRIVATE sigilvm ncurses xxHash::xxhash)

add_executable(sigilvm-xorit  ${CMAKE_SOURCE_DIR}/apps/xorit.cpp)
target_link_libraries(sigilvm-xorit PRIVATE sigilvm xxHash::xxhash)

add_executable(sigilvm-dotexe  ${CMAKE_SOURCE_DIR}/apps/dotexe.cpp)
target_link_libraries(sigilvm-dotexe PRIVATE sigilvm)
