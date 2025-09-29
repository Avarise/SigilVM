find_package(Vulkan REQUIRED)
find_package(Threads REQUIRED)
find_package(glfw3 REQUIRED)

if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    set(SIGIL_EXTRA_LIBS ${X11_LIBRARIES} Xcursor Xrandr Xi Xxf86vm)
endif()
