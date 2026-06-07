#pragma once

#include <vulkan/vulkan_core.h>

namespace sigil::render {

struct frame_t {
    uint32_t index;
    VkCommandBuffer cmdbf;
    VkCommandPool cmfpool;
    VkSemaphore image_acquired;
    VkSemaphore render_finished;
    VkFence fence;
};



}