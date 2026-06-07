#pragma once

#include <vulkan/vulkan_core.h>
#include <cstdint>

namespace sigil::platform {
    
struct gpu_info_t {
    char name[256];
    uint32_t vendor_id;
    uint32_t device_id;
    VkPhysicalDeviceType type;
    uint32_t vram_mb;
};

    

} // sigil::platform::device