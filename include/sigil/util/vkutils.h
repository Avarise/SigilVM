#pragma once

#include <sigil/platform/device.h>
#include <sigil/status.h>
#include <string>
#include <vector>

namespace sigil::util {
    status_t vk_probe_devices(std::vector<platform::gpu_info_t>& out_devices);
    status_t vk_list_extensions();
    std::string get_device_name(VkPhysicalDevice device);
}