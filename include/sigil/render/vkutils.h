#pragma once

#include <sigil/platform/device.h>
#include <thaumaturgy/thaumaturgy.h>
#include <string>
#include <vector>

namespace sigil::render {
    ::thaumaturgy::yield vk_probe_devices(std::vector<platform::gpu_info_t>& out_devices);
    ::thaumaturgy::yield vk_list_extensions();
    std::string get_device_name(VkPhysicalDevice device);
}