#pragma once
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <string>

#include "../core/status.h"

namespace sigilvm::platform {
    
struct window_t {
    GLFWwindow* handle = nullptr;
    int32_t width = 1920;
    int32_t height = 1080;
    std::string title = "Undefined";
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    
    ::sigilvm::status_t initialize(std::string title);
    ::sigilvm::status_t deinitialize();
};
    
 
}