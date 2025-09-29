#include "sigil/util/vkutils.h"
#include "sigil/status.h"
#include <cstdio>
#include <cstring>
#include <iostream>

namespace sigil::util {

status_t vk_list_extensions() {
    uint32_t extension_count = 0;
    VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    if (result != VK_SUCCESS) {
        std::cerr << "[Sigil::Vulkan] Failed to enumerate extension count (VkResult=" << result << ")\n";
        return sigil::VM_FAILED;
    }

    std::vector<VkExtensionProperties> extensions(extension_count);
    result = vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

    if (result != VK_SUCCESS) {
        std::cerr << "[Sigil::Vulkan] Failed to enumerate extensions (VkResult=" << result << ")\n";
        return sigil::VM_FAILED;
    }

    std::cout << "=== Vulkan Instance Extensions (" << extension_count << ") ===\n";
    for (const auto& ext : extensions)
        std::cout << " - " << ext.extensionName << " (spec version " << ext.specVersion << ")\n";

    std::cout << "=========================================\n";

    return sigil::VM_OK;
}

status_t vk_probe_devices(std::vector<gpu_info_t> &out_devices) {
    out_devices.clear();
    VkResult result;

    // --- Step 1: Minimal Vulkan instance creation ---
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Sigil GPU Probe";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Sigil";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo inst_info{};
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pApplicationInfo = &app_info;

    VkInstance instance = VK_NULL_HANDLE;
    result = vkCreateInstance(&inst_info, nullptr, &instance);
    if (result != VK_SUCCESS) {
        return sigil::VM_FAILED | sigil::VM_DEPENDENCY_MISSING;
    }

    // --- Step 2: Enumerate physical devices ---
    uint32_t device_count = 0;
    result = vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    if (result != VK_SUCCESS || device_count == 0) {
        vkDestroyInstance(instance, nullptr);
        return VM_FAILED;
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    result = vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    if (result != VK_SUCCESS) {
        vkDestroyInstance(instance, nullptr);
        return VM_FAILED;
    }

    // --- Step 3: Collect info about each GPU ---
    out_devices.reserve(device_count);
    for (VkPhysicalDevice dev : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(dev, &props);

        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(dev, &mem_props);

        gpu_info_t info{};
        //std::strncpy(info.name, props.deviceName, sizeof(info.name) - 1);
        std::snprintf(info.name, sizeof(info.name), "%s", props.deviceName);
        info.vendor_id = props.vendorID;
        info.device_id = props.deviceID;
        info.type = props.deviceType;

        // Approximate VRAM size (sum of device-local heaps)
        uint64_t vram_bytes = 0;
        for (uint32_t i = 0; i < mem_props.memoryHeapCount; ++i) {
            if (mem_props.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                vram_bytes += mem_props.memoryHeaps[i].size;
        }
        info.vram_mb = vram_bytes / (1024 * 1024);

        out_devices.push_back(info);
    }

    // --- Step 4: Cleanup ---
    vkDestroyInstance(instance, nullptr);

    return VM_OK;
} // vk_probe_devices

std::string get_device_name(VkPhysicalDevice device){
    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(device, &props);

    std::string name = props.deviceName;
    return name;
}

} // sigil


