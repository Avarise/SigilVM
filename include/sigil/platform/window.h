#pragma once

#include <vulkan/vulkan_core.h>
#include <sigil/status.h>
#include <GLFW/glfw3.h>
#include <string>

namespace sigil::platform {
    
struct window_t {
    GLFWwindow* handle = nullptr;
    int32_t width = 1920;
    int32_t height = 1080;
    std::string title = "Undefined";
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    
    status_t initialize(std::string title);
    status_t deinitialize();
};
    
 
}