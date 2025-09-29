#pragma once

#include <thaumaturgy/thaumaturgy.h>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <string>

namespace sigil::platform {
    
struct window_t {
    GLFWwindow* handle = nullptr;
    int32_t width = 1920;
    int32_t height = 1080;
    std::string title = "Undefined";
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    
    ::thaumaturgy::yield initialize(std::string title);
    ::thaumaturgy::yield deinitialize();
};
    
 
}