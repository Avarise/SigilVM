#pragma once
#include <sigil/platform/app.h>
#include <vulkan/vulkan.h>
#include <sigil/status.h>
#include <GLFW/glfw3.h>
#include <cstdint>

namespace sigil::render {

struct context_t {
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkCommandPool command_pool = VK_NULL_HANDLE;
    VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
    uint32_t graphics_queue_family = 0;
    VkAllocationCallbacks* allocator = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug_messenger;
    bool validation_enabled = false;

    
    status_t initialize_for_graphics();
    status_t initialize_for_compute();
    status_t shutdown();

    context_t(platform::app_descriptor_t dsc);
    ~context_t();
};


}