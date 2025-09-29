if(NOT BUILD_EXAMPLES)
    return()
endif()

add_executable(sigilvm-editor ${CMAKE_SOURCE_DIR}/apps/editor.cpp)
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
