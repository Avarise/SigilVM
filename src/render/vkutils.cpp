#include <sigil/platform/capabilities.h>
#include <sigil/render/vkutils.h>
#include <thaumaturgy/thaumaturgy.h>
#include <iostream>
#include <cstdio>

namespace sigil::render {

::thaumaturgy::yield vk_list_extensions() {
    using namespace ::thaumaturgy;

    uint32_t extension_count = 0;

    VkResult result =
        vkEnumerateInstanceExtensionProperties(
            nullptr, &extension_count, nullptr);

    if (result != VK_SUCCESS) {
        std::cerr << "[Sigil::Vulkan] Failed to enumerate extension count "
                        "(VkResult=" << result << ")\n";

        return yield(yield_state::fail)
            .set_origin(yield_origin::process)
            .set_code(static_cast<std::uint32_t>(result));
    }

    std::vector<VkExtensionProperties> extensions(extension_count);

    result =
        vkEnumerateInstanceExtensionProperties(
            nullptr, &extension_count, extensions.data());

    if (result != VK_SUCCESS) {
        std::cerr << "[Sigil::Vulkan] Failed to enumerate extensions "
                        "(VkResult=" << result << ")\n";

        return yield(yield_state::fail)
            .set_origin(yield_origin::process)
            .set_code(static_cast<std::uint32_t>(result));
    }

    std::cout << "=== Vulkan Instance Extensions ("
                << extension_count << ") ===\n";

    for (const auto& ext : extensions)
        std::cout << " - " << ext.extensionName
                    << " (spec version " << ext.specVersion << ")\n";

    std::cout << "=========================================\n";

    return {}; // OK
}

::thaumaturgy::yield vk_probe_devices(std::vector<platform::gpu_info_t> &out_devices) {
    out_devices.clear();
    using namespace ::thaumaturgy;
    VkResult result;

    // Minimal Vulkan instance creation
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
        return yield(yield_state::fail)
            .set_origin(yield_origin::process)
            .set_code(static_cast<std::uint32_t>(result));
    }

    // Enumerate physical devices
    uint32_t device_count = 0;
    result = vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    if (result != VK_SUCCESS || device_count == 0) {
        vkDestroyInstance(instance, nullptr);
        return yield(yield_state::fail)
            .set_origin(yield_origin::process)
            .set_code(static_cast<std::uint32_t>(result));
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    result = vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    if (result != VK_SUCCESS) {
        vkDestroyInstance(instance, nullptr);
        return yield(yield_state::fail)
            .set_origin(yield_origin::process)
            .set_code(static_cast<std::uint32_t>(result));
    }

    // Collect info about each GPU
    out_devices.reserve(device_count);
    for (VkPhysicalDevice dev : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(dev, &props);

        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(dev, &mem_props);

        platform::gpu_info_t info{};
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

    // Cleanup
    vkDestroyInstance(instance, nullptr);

    return {};
} // vk_probe_devices

std::string get_device_name(VkPhysicalDevice device){
    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(device, &props);

    std::string name = props.deviceName;
    return name;
}

} // sigil::render
