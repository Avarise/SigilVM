#pragma once

#include <vulkan/vulkan.h>
#include <sigil/status.h>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <vector>
#include <string>


namespace sigil {

struct gpu_info_t {
    char name[256];
    uint32_t vendor_id;
    uint32_t device_id;
    VkPhysicalDeviceType type;
    uint32_t vram_mb;
};

struct frame_t {
    uint32_t index;
    VkCommandBuffer cmdbf;
    VkCommandPool cmfpool;
    VkSemaphore image_acquired;
    VkSemaphore render_finished;
    VkFence fence;
};

struct frame_semaphore_t
{
    VkSemaphore         img_acq;
    VkSemaphore         img_render_complete;
};


struct render_ctx_init_info_t {
    bool enable_validation;
    bool enable_glfw_support;
};

struct window_t {


};


struct window_surface_t {
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkFormat format;
    VkExtent2D extent;
    VkRenderPass render_pass;
    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
    std::vector<VkFramebuffer> framebuffers;
    uint32_t image_index;

    bool create_surface(VkInstance instance, void* glfw_window);
    bool create_swapchain(VkDevice device, VkPhysicalDevice physical_device, uint32_t queue_family_index, bool vsync);
    void recreate_swapchain(VkDevice device, VkPhysicalDevice physical_device, uint32_t queue_family_index, bool vsync);
    void destroy(VkDevice device);
};

struct frame_resources_t {
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    VkSemaphore image_acquired;
    VkSemaphore render_finished;
    VkFence in_flight;

    void init(VkDevice device, uint32_t queue_family);
    void destroy(VkDevice device);
};

struct pipeline_t {
    VkPipelineLayout layout;
    VkPipeline pipeline;
    VkShaderModule vert_shader;
    VkShaderModule frag_shader;

    bool create(VkDevice device, VkRenderPass render_pass, const char* vert_path, const char* frag_path);
    void destroy(VkDevice device);
};

struct shader_stage_t {
    VkShaderStageFlagBits stage;
    VkShaderModule module;
};

void glfw_error_callback(int error, const char* description);


} // namespace sigil
