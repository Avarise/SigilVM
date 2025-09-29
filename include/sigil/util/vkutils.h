#pragma once

#include <sigil/render/device.h>
#include <sigil/status.h>
#include <vector>

namespace sigil::util {
    status_t vk_probe_devices(std::vector<gpu_info_t>& out_devices);
    status_t vk_list_extensions();
    std::string get_device_name(VkPhysicalDevice device);
}