#include "sigil/macros.h"
#include "thaumaturgy/yield.h"
#include <sigil/platform/app.h>
#include <sigil/render/context.h>
#include <thaumaturgy/thaumaturgy.h>
#include <iostream>
#include <cstring>
#include <ostream>
#include <vector>

namespace sigil::render {

context_t::context_t(const platform::app_descriptor_t desc) {
#   ifdef SIGIL_DEBUG
    this->validation_enabled = true;
#   else
    this->validation_enabled = false;
#   endif

    SIGIL_UNUSED(desc);



// TODO: Move all complex init outside of constructor
//     // Variable init, so that GOTO does not fight me
//     VkApplicationInfo app_info{};
//     VkResult result = VK_SUCCESS;
//     uint32_t available_ext_count = 0;
//     VkInstanceCreateInfo inst_info{};






//     // Create Vulkan App Info struct and result var

//     app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
//     app_info.pApplicationName = "SigilVM Renderer";
//     app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
//     app_info.pEngineName = "SigilVM";
//     app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
//     app_info.apiVersion = VK_API_VERSION_1_3;

//     // Prepare Extensions & Layers
//     std::vector<const char*> extensions;
//     std::vector<const char*> validation_layers;
//     extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);



// #   if defined(__linux__)
//         //extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
// #   elif defined(_WIN32)
//         //extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
// #   endif

//     // Check extension support

//     vkEnumerateInstanceExtensionProperties(nullptr, &available_ext_count, nullptr);
//     std::vector<VkExtensionProperties> available_exts(available_ext_count);
//     vkEnumerateInstanceExtensionProperties(nullptr, &available_ext_count, available_exts.data());

//     for (auto& req : extensions) {
//         bool found = false;

//         for (auto& avail : available_exts) {
//             if (strcmp(req, avail.extensionName) == 0) {
//                 found = true;
//                 break;
//             }
//         }

//         if (!found) {
//             std::cerr << "[ERROR] Missing VK Extension: " << req << std::endl;
//             return;
//         }
//     }


//     // Creation of Vulkan Instance

//     inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//     inst_info.pApplicationInfo = &app_info;
//     inst_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
//     inst_info.ppEnabledExtensionNames = extensions.data();
//     inst_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
//     inst_info.ppEnabledLayerNames = validation_layers.data();

//     result = vkCreateInstance(&inst_info, allocator, &instance);
//     if (result != VK_SUCCESS) {
//         std::cerr << "[ERROR] Failed to create Vulkan instance (code " << result << ")\n";
//         goto err_cleanup;
//     }


//     // Enumerate physical devics
//     uint32_t device_count = 0;
//     vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
//     if (device_count == 0) {
//         std::cerr << "[Sigil-Vulkan] No Vulkan-capable GPU found.\n";
//         return;
//     }

//     std::vector<VkPhysicalDevice> devices(device_count);
//     vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

//     // For simplicity, pick the first discrete GPU available.
//     for (auto& dev : devices) {
//         VkPhysicalDeviceProperties props;
//         vkGetPhysicalDeviceProperties(dev, &props);
//         if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
//             physical_device = dev;
//             std::cout << "Using GPU: " << props.deviceName << "\n";
//             break;
//         }
//     }
//     if (physical_device == VK_NULL_HANDLE) physical_device = devices[0];

//     // -------------------------------------------------------------------------
//     // Step 3: Queue Family Selection
//     // -------------------------------------------------------------------------
//     uint32_t queue_family_count = 0;
//     vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
//     std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
//     vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

//     int graphics_index = -1;
//     for (uint32_t i = 0; i < queue_family_count; ++i) {
//         if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
//             graphics_index = i;
//             break;
//         }
//     }

//     if (graphics_index == -1) {
//         std::cerr << "[Sigil-Vulkan] No suitable graphics queue found.\n";
//         return VM_RESOURCE_MISSING | VM_FAILED;
//     }

//     graphics_queue_family = graphics_index;

//     // -------------------------------------------------------------------------
//     // Step 4: Create Logical Device and Queue
//     // -------------------------------------------------------------------------
//     float queue_priority = 1.0f;
//     VkDeviceQueueCreateInfo queue_info{};
//     queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//     queue_info.queueFamilyIndex = graphics_queue_family;
//     queue_info.queueCount = 1;
//     queue_info.pQueuePriorities = &queue_priority;

//     VkDeviceCreateInfo dev_info{};
//     dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//     dev_info.queueCreateInfoCount = 1;
//     dev_info.pQueueCreateInfos = &queue_info;

//     const char* device_exts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
//     if (enable_glfw_support) {
//         dev_info.enabledExtensionCount = 1;
//         dev_info.ppEnabledExtensionNames = device_exts;
//     }

//     result = vkCreateDevice(physical_device, &dev_info, allocator, &device);
//     if (result != VK_SUCCESS) {
//         std::cerr << "[Sigil-Vulkan] Failed to create logical device.\n";
//         return VM_FAILED;
//     }

//     vkGetDeviceQueue(device, graphics_queue_family, 0, &graphics_queue);
//     std::cout << "Sigil-Vulkan: Logical device and graphics queue ready.\n";


//     std::cout << "[INFO] SigilVM Renderer started" << std::endl;
//     return;

//     ///Error handling and cleanup here
//     err_cleanup:


//     return;
}

context_t::~context_t() {
    //std::cout << "Removing Render Context" << std::endl;
}

::thaumaturgy::yield context_t::initialize_for_compute() {
    ::thaumaturgy::yield status;

    return status;
}

::thaumaturgy::yield context_t::initialize_for_graphics() {
    ::thaumaturgy::yield status;

    return status;
}

} // namespace sigil::render
