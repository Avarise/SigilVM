/**
 * @file editor.cpp
 * @author Sebastian Czapla (https://github.com/Avarise)
 * @brief SigilVM Editor (sigilvm-extra package)
 * @version 0.1
 * @date 2025-12-02
 *
 * @copyright Copyright (c) 2026
 *
 * Asset, text, scene editor for SigilVM Game Engine
 */

#include <sigil/platform/context.h>
#include <sigil/platform/process.h>
#include <sigil/platform/compat.h>
#include <sigil/network/context.h>
#include <sigil/game/initiative.h>
#include <sigil/render/context.h>
#include <sigil/render/vkutils.h>
#include <sigil/media/context.h>
#include <sigil/platform/paths.h>
#include <sigil/platform/glfw.h>
#include <sigil/platform/exec.h>
#include <sigil/platform/app.h>
#include <sigil/vm/instance.h>
#include <sigil/math/vector.h>
#include <sigil/vm/context.h>
#include <sigil/common.h>

#include <GLFW/glfw3.h>
#include <filesystem>
#include <algorithm>
#include <stdlib.h>
#include <fstream>
#include <stdio.h>
#include <imgui.h>
#include <csignal>
#include <ostream>
#include <vector>
#include <atomic>

// --- LOCAL DEFINITIONS ---
struct output_line_t {
    std::string text;
    ::sigil::math::vector_t<float, 4> color;
    //ImVec4 color;
};

enum class file_action_type {
    open,
    rename,
    delete_,
    reveal_in_system,
    run_exe,
    none
};

struct file_action_t {
    file_action_type type;
    std::string label;
};

struct text_editor_document_t {
    std::filesystem::path path;
    std::string buffer;
    bool open = true;
    bool dirty = false;
};

// --- APP-LOCAL (STATIC) VARIABLES ---
// SigilVM identity and state
static std::atomic<bool> g_shutdown_requested{false};
static sigil::platform::process_descriptor_t proc_info;
static sigil::platform::app_descriptor_t app_info;

// SigilVM subsystem containers / contexts
static sigil::platform::context_t platform_ctx;
static sigil::network::context_t network_ctx;
static sigil::render::context_t render_ctx;
static sigil::media::context_t media_ctx;
static sigil::vm::context_t vm_ctx;

// Log container
static struct {
    std::vector<output_line_t> lines;
    bool scroll_to_bottom = false;
} log_output;

// Data for compatibilty modal (ImGui subwindow)
static struct {
    std::vector<sigil::platform::compat_tool_t> tools;
    std::vector<std::filesystem::path> prefixes;
    std::filesystem::path target;
    bool open = false;

    int selected_prefix = -1;
    int selected_tool = -1;

} compat_modal;

// File Explorer
static struct {
    std::filesystem::path path;
    bool valid = false;
} explorer_selection;

// Console
static struct {
    std::vector<std::string> lines;
    std::string input_buffer;

    bool scroll_to_bottom = false;
} console;

static struct {
    std::vector<text_editor_document_t> documents;
    int active_index = -1;
} text_editor;

// Panel Visibility
static bool show_file_explorer = false;
static bool show_style_editor = false;
static bool show_level_editor = false;
static bool show_asset_manager = true;
static bool show_image_viewer = false;
static bool show_imgui_demo = false;
static bool show_text_editor = true;
static bool show_console = false;
static bool show_preview = true;
static bool show_output = true;
static bool show_character_editor = false;
static bool show_initiative_tracker = true;

// Something??
static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

//#define APP_USE_UNLIMITED_FRAME_RATE
// Window, monitor, fullscreen, rendering, vulkan
#ifdef SIGIL_DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
#endif

static GLFWwindow* g_window = nullptr;
static bool g_fullscreen = false;
static VkAllocationCallbacks*   g_Allocator = nullptr;
static VkInstance               g_Instance = VK_NULL_HANDLE;
static VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice                 g_Device = VK_NULL_HANDLE;
static uint32_t                 g_QueueFamily = (uint32_t)-1;
static VkQueue                  g_Queue = VK_NULL_HANDLE;
static VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;
static ImGui_ImplVulkanH_Window g_MainWindowData;
static uint32_t                 g_MinImageCount = 2;
static bool                     g_SwapChainRebuild = false;

// Handler to exit cleanly on the signals
extern "C" void signal_handler(int signo) {
    if (signo == SIGINT || signo == SIGTERM) {
        g_shutdown_requested.store(true);
    }
}

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void check_vk_result(VkResult err) {
    if (err == VK_SUCCESS)
        return;
    fprintf(stderr, "[vulkan] Error: %d - %s\n", err, sigil::render::vkresult_to_string(err).c_str());
    if (err < 0)
        abort();
}


// Commands for dispatcher
static ::sigil::yield cmd_configure(const ::sigil::platform::cmd_handler_args_t handler_args);
static ::sigil::yield cmd_help(const ::sigil::platform::cmd_handler_args_t handler_args);
static ::sigil::yield cmd_gui(const ::sigil::platform::cmd_handler_args_t handler_args);

// Main GUI draw
static void draw_gui();

// Sub-windows
static void draw_file_explorer();
static void draw_asset_manager();
static void draw_style_editor();
static void draw_level_editor();
static void draw_image_viewer();
static void draw_text_editor();
static void draw_imgui_demo();
static void draw_preview();
static void draw_console();
static void draw_output();
static void draw_compat();

// Logging functions
static void log_error(const std::string& msg);
static void log_info(const std::string& msg);
static void log_warn(const std::string& msg);
static void log_debug(const std::string& msg);

// Text Editor
static int text_editor_input_callback(ImGuiInputTextCallbackData* data);
static void save_text_editor_document(text_editor_document_t& doc);
static void open_text_editor(const std::filesystem::path& path);

static void open_text_editor(const std::filesystem::path& path);
static std::vector<file_action_t> resolve_actions(const std::filesystem::path& path);
static void dispatch_file_action(file_action_type action, const std::filesystem::path& path);
static int input_text_callback(ImGuiInputTextCallbackData* data);
static void read_fd_into_lines(int fd);
static void execute_command(const std::string& cmd);



GLFWmonitor* GetCurrentMonitor(GLFWwindow* window)
{
    int wx, wy, ww, wh;
    glfwGetWindowPos(window, &wx, &wy);
    glfwGetWindowSize(window, &ww, &wh);

    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

    GLFWmonitor* bestMonitor = monitors[0];
    int bestOverlap = 0;

    for (int i = 0; i < monitorCount; ++i)
    {
        int mx, my;
        glfwGetMonitorPos(monitors[i], &mx, &my);

        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        int mw = mode->width;
        int mh = mode->height;

        int overlapWidth  = std::max(0, std::min(wx + ww, mx + mw) - std::max(wx, mx));
        int overlapHeight = std::max(0, std::min(wy + wh, my + mh) - std::max(wy, my));
        int overlapArea   = overlapWidth * overlapHeight;

        if (overlapArea > bestOverlap)
        {
            bestOverlap = overlapArea;
            bestMonitor = monitors[i];
        }
    }

    return bestMonitor;
}

GLFWmonitor* ResolveTargetMonitor(GLFWwindow* window)
{
    // If already fullscreen, GLFW knows the monitor
    if (GLFWmonitor* m = glfwGetWindowMonitor(window))
        return m;

#if defined(GLFW_PLATFORM_X11)
    // Use overlap heuristic (your previous function)
    return GetCurrentMonitor(window);
#else
    // Wayland fallback
    return g_last_monitor ? g_last_monitor : glfwGetPrimaryMonitor();
#endif
}


void SetBorderlessFullscreen(GLFWwindow* window, bool enabled)
{
    if (!g_window) return;

    GLFWmonitor* monitor = GetCurrentMonitor(g_window);
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    if (enabled)
    {
        glfwSetWindowMonitor(
            window,
            monitor,   // <-- key difference
            0, 0,
            mode->width,
            mode->height,
            mode->refreshRate
        );
    }
    else
    {
        glfwSetWindowMonitor(
            window,
            nullptr,
            100, 100,
            1280, 800,
            0
        );
    }
}


// ImGui ripped rendering

#ifdef APP_USE_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    (void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
    fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
    return VK_FALSE;
}
#endif // APP_USE_VULKAN_DEBUG_REPORT

static bool IsExtensionAvailable(const ImVector<VkExtensionProperties>& properties, const char* extension)
{
    for (const VkExtensionProperties& p : properties)
        if (strcmp(p.extensionName, extension) == 0)
            return true;
    return false;
}

static void SetupVulkan(ImVector<const char*> instance_extensions)
{
    VkResult err;
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
    volkInitialize();
#endif

    // Create Vulkan Instance
    {
        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        // Enumerate available extensions
        uint32_t properties_count;
        ImVector<VkExtensionProperties> properties;
        vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
        properties.resize(properties_count);
        err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.Data);
        check_vk_result(err);

        // Enable required extensions
        if (IsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
            instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
        if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
        {
            instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }
#endif

        // Enabling validation layers
#ifdef APP_USE_VULKAN_DEBUG_REPORT
        const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = layers;
        instance_extensions.push_back("VK_EXT_debug_report");
#endif

        // Create Vulkan Instance
        create_info.enabledExtensionCount = (uint32_t)instance_extensions.Size;
        create_info.ppEnabledExtensionNames = instance_extensions.Data;
        err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
        check_vk_result(err);
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
        volkLoadInstance(g_Instance);
#endif

        // Setup the debug report callback
#ifdef APP_USE_VULKAN_DEBUG_REPORT
        auto f_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkCreateDebugReportCallbackEXT");
        IM_ASSERT(f_vkCreateDebugReportCallbackEXT != nullptr);
        VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
        debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debug_report_ci.pfnCallback = debug_report;
        debug_report_ci.pUserData = nullptr;
        err = f_vkCreateDebugReportCallbackEXT(g_Instance, &debug_report_ci, g_Allocator, &g_DebugReport);
        check_vk_result(err);
#endif
    }

    // Select Physical Device (GPU)
    g_PhysicalDevice = ImGui_ImplVulkanH_SelectPhysicalDevice(g_Instance);
    IM_ASSERT(g_PhysicalDevice != VK_NULL_HANDLE);

    // Select graphics queue family
    g_QueueFamily = ImGui_ImplVulkanH_SelectQueueFamilyIndex(g_PhysicalDevice);
    IM_ASSERT(g_QueueFamily != (uint32_t)-1);

    // Create Logical Device (with 1 queue)
    {
        ImVector<const char*> device_extensions;
        device_extensions.push_back("VK_KHR_swapchain");

        // Enumerate physical device extension
        uint32_t properties_count;
        ImVector<VkExtensionProperties> properties;
        vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr, &properties_count, nullptr);
        properties.resize(properties_count);
        vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr, &properties_count, properties.Data);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
        if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
            device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

        const float queue_priority[] = { 1.0f };
        VkDeviceQueueCreateInfo queue_info[1] = {};
        queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[0].queueFamilyIndex = g_QueueFamily;
        queue_info[0].queueCount = 1;
        queue_info[0].pQueuePriorities = queue_priority;
        VkDeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
        create_info.pQueueCreateInfos = queue_info;
        create_info.enabledExtensionCount = (uint32_t)device_extensions.Size;
        create_info.ppEnabledExtensionNames = device_extensions.Data;
        err = vkCreateDevice(g_PhysicalDevice, &create_info, g_Allocator, &g_Device);
        check_vk_result(err);
        vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
    }

    // Create Descriptor Pool
    // If you wish to load e.g. additional textures you may need to alter pools sizes and maxSets.
    {
        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE },
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 0;
        for (VkDescriptorPoolSize& pool_size : pool_sizes)
            pool_info.maxSets += pool_size.descriptorCount;
        pool_info.poolSizeCount = (uint32_t)IM_COUNTOF(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        err = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator, &g_DescriptorPool);
        check_vk_result(err);
    }
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
static void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height)
{
    // Check for WSI support
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily, surface, &res);
    if (res != VK_TRUE)
    {
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }

    // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->Surface = surface;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_COUNTOF(requestSurfaceImageFormat), requestSurfaceColorSpace);

    // Select Present Mode
#ifdef APP_USE_UNLIMITED_FRAME_RATE
    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(g_PhysicalDevice, wd->Surface, &present_modes[0], IM_COUNTOF(present_modes));
    //printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(g_MinImageCount >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, wd, g_QueueFamily, g_Allocator, width, height, g_MinImageCount, 0);
}

static void CleanupVulkan()
{
    vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);

#ifdef APP_USE_VULKAN_DEBUG_REPORT
    // Remove the debug report callback
    auto f_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkDestroyDebugReportCallbackEXT");
    f_vkDestroyDebugReportCallbackEXT(g_Instance, g_DebugReport, g_Allocator);
#endif // APP_USE_VULKAN_DEBUG_REPORT

    vkDestroyDevice(g_Device, g_Allocator);
    vkDestroyInstance(g_Instance, g_Allocator);
}

static void CleanupVulkanWindow(ImGui_ImplVulkanH_Window* wd) {
    ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, wd, g_Allocator);
    vkDestroySurfaceKHR(g_Instance, wd->Surface, g_Allocator);
}

static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data) {
    VkSemaphore image_acquired_semaphore  = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkResult err = vkAcquireNextImageKHR(g_Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        g_SwapChainRebuild = true;
    if (err == VK_ERROR_OUT_OF_DATE_KHR)
        return;
    if (err != VK_SUBOPTIMAL_KHR)
        check_vk_result(err);

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
        err = vkWaitForFences(g_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
        check_vk_result(err);

        err = vkResetFences(g_Device, 1, &fd->Fence);
        check_vk_result(err);
    }
    {
        err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        check_vk_result(err);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        check_vk_result(err);
        err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
        check_vk_result(err);
    }
}

static void FramePresent(ImGui_ImplVulkanH_Window* wd) {
    if (g_SwapChainRebuild) return;

    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(g_Queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        g_SwapChainRebuild = true;
    if (err == VK_ERROR_OUT_OF_DATE_KHR)
        return;
    if (err != VK_SUBOPTIMAL_KHR)
        check_vk_result(err);
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount; // Now we can use the next set of semaphores
}



int main(const int argc, const char **argv, const char **envp) {
    sigil::platform::command_registry_t reg;
    ::sigil::yield st = {};

    // App identity
    st = sigil::platform::process_initialize(proc_info, argc, argv, envp);
    if (st.is_failure()) {
        std::cout << "Failed to register editor commands" << std::endl;
        return -1;
    }

    st = sigil::platform::app_initialize(app_info, proc_info);
    if (st.is_failure()) {
        std::cout << "Failed to register editor commands" << std::endl;
        return -1;
    }

    // Command registration
    std::vector<::sigil::platform::command_t> cmds {
        ::sigil::platform::command_t({                      }, false, cmd_gui),
        ::sigil::platform::command_t({ "help"               }, true,  cmd_help),
        ::sigil::platform::command_t({ "configure"          }, true,  cmd_configure),
    };

    st = ::sigil::platform::register_command(reg, cmds);
    if (st.is_failure()) {
        std::cout << "Failed to register editor commands" << std::endl;
        return -1;
    }

    st = sigil::platform::dispatch_command(reg, argc, argv);
    if (st.is_failure()) {
        std::cout << "Failed to dispatch editor command" << std::endl;
    }

    return st.code;
}

static ::sigil::yield cmd_help(const ::sigil::platform::cmd_handler_args_t handler_args) {
    SIGIL_UNUSED(handler_args);
    std::cout << "Run sigilvm-editor to start GUI" << std::endl;
    return {};
}

static ::sigil::yield cmd_configure(const ::sigil::platform::cmd_handler_args_t handler_args) {
    SIGIL_UNUSED(handler_args);
    std::cout << "CLI Config not yet implemented" << std::endl;
    return {};
}

static ::sigil::yield cmd_gui(const ::sigil::platform::cmd_handler_args_t handler_args) {
    ::sigil::yield ret;
    SIGIL_UNUSED(handler_args);

    app_info.log_info = log_info;
    app_info.log_warn = log_warn;
    app_info.log_error = log_error;
    app_info.log_debug = log_debug;

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return ret.set_state(sigil::yield_state::fail);

    // Create window with Vulkan context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    g_window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "SigilVM Editor", nullptr, nullptr);
    if (!glfwVulkanSupported()) {
        printf("GLFW: Vulkan Not Supported\n");
        return ret.set_state(sigil::yield_state::fail);
    }

    ImVector<const char*> extensions;
    uint32_t extensions_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
    for (uint32_t i = 0; i < extensions_count; i++)
        extensions.push_back(glfw_extensions[i]);
    SetupVulkan(extensions);

    // Create Window Surface
    VkSurfaceKHR surface;
    VkResult err = glfwCreateWindowSurface(g_Instance, g_window, g_Allocator, &surface);
    check_vk_result(err);

    // Create Framebuffers
    int w, h;
    glfwGetFramebufferSize(g_window, &w, &h);
    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
    SetupVulkanWindow(wd, surface, w, h);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //static std::string ini_path = get_editor_ini_path();
    auto ini_path = ::sigil::platform::get_local_config(app_info) / "editor.ini";

    std::cout << "Config path: " << ini_path.string() << std::endl;

    io.IniFilename = ini_path.c_str();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsClassic();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)
    io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
    io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(g_window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    //init_info.ApiVersion = VK_API_VERSION_1_3;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
    init_info.Instance = g_Instance;
    init_info.PhysicalDevice = g_PhysicalDevice;
    init_info.Device = g_Device;
    init_info.QueueFamily = g_QueueFamily;
    init_info.Queue = g_Queue;
    init_info.PipelineCache = g_PipelineCache;
    init_info.DescriptorPool = g_DescriptorPool;
    init_info.MinImageCount = g_MinImageCount;
    init_info.ImageCount = wd->ImageCount;
    init_info.Allocator = g_Allocator;
    init_info.PipelineInfoMain.RenderPass = wd->RenderPass;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);

    style.FontSizeBase = 17.0f;

    ImFont* default_font = io.Fonts->AddFontFromFileTTF(
        "/usr/share/fonts/TTF/0xProtoNerdFont-Regular.ttf",
        16.0f
    );

    io.FontDefault = default_font;

    log_info("SigilVM Editor Started");

    // Main loop
    while (!glfwWindowShouldClose(g_window) && !g_shutdown_requested.load(std::memory_order_acquire)) {
        glfwPollEvents();

        // Resize swap chain?
        int fb_width, fb_height;
        glfwGetFramebufferSize(g_window, &fb_width, &fb_height);

        if (fb_width > 0 && fb_height > 0 && (g_SwapChainRebuild || g_MainWindowData.Width != fb_width || g_MainWindowData.Height != fb_height)) {
            ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
            ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, wd, g_QueueFamily, g_Allocator, fb_width, fb_height, g_MinImageCount, 0);
            g_MainWindowData.FrameIndex = 0;
            g_SwapChainRebuild = false;
        }

        if (glfwGetWindowAttrib(g_window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        draw_gui();
        ImGui::Render();
        ImDrawData* main_draw_data = ImGui::GetDrawData();
        const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
        wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
        wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
        wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
        wd->ClearValue.color.float32[3] = clear_color.w;
        if (!main_is_minimized)
            FrameRender(wd, main_draw_data);

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        // Present Main Platform Window
        if (!main_is_minimized)
            FramePresent(wd);
    }

    // Cleanup
    err = vkDeviceWaitIdle(g_Device);
    check_vk_result(err);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    CleanupVulkanWindow(&g_MainWindowData);
    CleanupVulkan();

    glfwDestroyWindow(g_window);
    glfwTerminate();
    return {};
}

static void log_info(const std::string& msg){
    log_output.lines.push_back({
        "[info] " + msg,
        ::sigil::math::vector_t<float,4>{0.7f, 0.7f, 0.7f, 1.0f}
    });
    log_output.scroll_to_bottom = true;
}

static void log_warn(const std::string& msg) {
    log_output.lines.push_back({
        "[warn] " + msg,
        ::sigil::math::vector_t<float,4>{0.9f, 0.7f, 0.2f, 1.0f}
    });

    log_output.scroll_to_bottom = true;
}

static void log_error(const std::string& msg) {
    log_output.lines.push_back({
        "[error] " + msg,
        ::sigil::math::vector_t<float,4>{0.9f, 0.2f, 0.2f, 1.0f}
    });

    log_output.scroll_to_bottom = true;
}

static void log_debug(const std::string& msg) {
    log_output.lines.push_back({
        "[debug] " + msg,
        ::sigil::math::vector_t<float,4>{0.3f, 0.7f, 1.0f, 1.0f}
    });

    log_output.scroll_to_bottom = true;
}

static std::vector<file_action_t> resolve_actions(const std::filesystem::path& path) {
    std::vector<file_action_t> actions;

    if (!std::filesystem::exists(path))
        return actions;

    if (path.extension() == ".exe") {
        actions.push_back({ file_action_type::run_exe, "Run (.exe)" });
    }


    if (std::filesystem::is_regular_file(path)) {
        actions.push_back({ file_action_type::open, "Open" });
        actions.push_back({ file_action_type::rename, "Rename" });
        actions.push_back({ file_action_type::delete_, "Delete" });
    }

    if (std::filesystem::is_directory(path)) {
        actions.push_back({ file_action_type::open, "Enter Directory" });
    }


    actions.push_back({ file_action_type::reveal_in_system, "Reveal in System" });

    return actions;
}

static void dispatch_file_action(file_action_type action, const std::filesystem::path& path) {
    switch (action) {
        case file_action_type::open: {
            log_info("Opening: " + path.string());
            open_text_editor(path);

            // extension-based dispatch (stub for now)
            // if (path.extension() == ".txt") {
            // } else {
            //     log_warn("No editor for file: " + path.string());
            // }

            break;
        }

        case file_action_type::run_exe:
        {
            compat_modal.open = true;
            compat_modal.target = path;

            compat_modal.prefixes.clear();
            compat_modal.tools.clear();

            // Load prefixes
            std::filesystem::path base =
                std::filesystem::path(std::getenv("HOME")) /
                ".local/share/sigilvm/wlx64/prefixes";

            for (auto& entry : std::filesystem::directory_iterator(base))
            {
                if (entry.is_directory())
                    compat_modal.prefixes.push_back(entry.path());
            }

            // Probe tools (Proton + Wine already handled inside)
            sigil::platform::probe_compat_tools(app_info, compat_modal.tools);

            compat_modal.selected_prefix = -1;
            compat_modal.selected_tool = -1;

            log_info("Preparing compat run: " + path.string());
            break;
        }

        case file_action_type::rename: {
            log_info("Rename requested: " + path.string());
            // later: trigger modal
            break;
        }

        case file_action_type::delete_: {
            log_warn("Delete requested: " + path.string());
            // later: confirmation modal
            break;
        }

        case file_action_type::reveal_in_system: {
            log_info("Reveal in system: " + path.string());
            // later: xdg-open or similar
            break;
        }

        default:
            break;
    }
}

static int input_text_callback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        std::string* str = (std::string*)data->UserData;
        str->resize(data->BufTextLen);
        data->Buf = str->data();
    }
    return 0;
}

static void read_fd_into_lines(int fd) {
    if (fd < 0) return;

    char buffer[4096];
    ssize_t n;

    std::string current;

    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < n; ++i) {
            if (buffer[i] == '\n') {
                console.lines.push_back(current);
                current.clear();
            }
            else {
                current += buffer[i];
            }
        }
    }

    if (!current.empty())
        console.lines.push_back(current);

    close(fd);
    console.scroll_to_bottom = true;
}

static void execute_command(const std::string& cmd) {
    using namespace sigil::platform;
    proc_exec_unit_t peu;

    peu.set_target("/bin/sh")
       .push_argument("-c")
       .push_argument(cmd)
       .set_exec_mode(EXEC_WAIT)
       .set_stdio_mode(STDIO_INHERIT, STDIO_CAPTURE, STDIO_CAPTURE);

    auto y = execute(peu);

    if (y.is_failure()) {
        console.lines.push_back("[error] execution failed");
        return;
    }

    ::read_fd_into_lines(peu.result.stdout_fd);
    ::read_fd_into_lines(peu.result.stderr_fd);
}

static int text_editor_input_callback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        std::string* str = (std::string*)data->UserData;
        str->resize(data->BufTextLen);
        data->Buf = str->data();
    }
    return 0;
}

static void open_text_editor(const std::filesystem::path& path) {
    // Check if already open
    for (size_t i = 0; i < text_editor.documents.size(); ++i) {
        if (text_editor.documents[i].path == path) {
            text_editor.active_index = (int)i;
            log_info("File already open: " + path.string());
            return;
        }
    }

    // Load file
    std::ifstream file(path);
    if (!file.is_open()) {
        log_error("Failed to open file: " + path.string());
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());

    text_editor_document_t doc;
    doc.path = path;
    doc.buffer = content;
    doc.open = true;
    doc.dirty = false;

    text_editor.documents.push_back(std::move(doc));
    text_editor.active_index = (int)text_editor.documents.size() - 1;

    log_info("Opened file: " + path.string());
}

static void save_text_editor_document(text_editor_document_t& doc) {
    std::ofstream file(doc.path);
    if (!file.is_open()) {
        log_error("Failed to save file: " + doc.path.string());
        return;
    }

    file << doc.buffer;
    doc.dirty = false;

    log_info("Saved file: " + doc.path.string());
}

static void draw_gui() {
    // Create a fullscreen invisible window that hosts the dockspace
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoDocking;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    window_flags |= ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                    ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("DockSpaceRoot", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    // --- Global shortcut handling ---
    ImGuiIO& io = ImGui::GetIO();
    if (io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_F4)) {
        g_shutdown_requested.store(true);
    }

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("SigilVM")) {
            if (ImGui::MenuItem("Plugins")) {}
            if (ImGui::MenuItem("Themes")) {}
            if (ImGui::MenuItem("Dotexe")) {}
            if (ImGui::MenuItem("Settings")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("About")) {}
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                g_shutdown_requested.store(true);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Project")) {
            if (ImGui::MenuItem("New Project")) {}
            if (ImGui::MenuItem("Open Project")) {}
            if (ImGui::MenuItem("Save Project")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Project Settings")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Create File")) {}
            if (ImGui::MenuItem("Open File")) {}
            if (ImGui::MenuItem("Save File")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo")) {}
            if (ImGui::MenuItem("Redo")) {}
            if (ImGui::MenuItem("Copy")) {}
            if (ImGui::MenuItem("Cut")) {}
            if (ImGui::MenuItem("Paste")) {}
            if (ImGui::MenuItem("Find")) {}
            if (ImGui::MenuItem("Replace")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Fullscreen", nullptr, g_fullscreen))
            {
                g_fullscreen = !g_fullscreen;
                SetBorderlessFullscreen(g_window, g_fullscreen);
            }
            if (ImGui::MenuItem("Reset Layout")) {}
            ImGui::Separator();
            ImGui::MenuItem("Asset Manager", nullptr, &show_asset_manager);
            ImGui::MenuItem("File Explorer", nullptr, &show_file_explorer);
            ImGui::Separator();
            ImGui::MenuItem("Output", nullptr, &show_output);
            ImGui::MenuItem("Preview", nullptr, &show_preview);
            ImGui::MenuItem("Terminal", nullptr, &show_console);
            ImGui::Separator();
            ImGui::MenuItem("Text Editor", nullptr, &show_text_editor);
            ImGui::MenuItem("Level Editor", nullptr, &show_level_editor);
            ImGui::MenuItem("Style Editor", nullptr, &show_style_editor);
            ImGui::MenuItem("Image Viewer", nullptr, &show_image_viewer);
            ImGui::MenuItem("ImGui Demo", nullptr, &show_imgui_demo);
            ImGui::Separator();
            ImGui::MenuItem("Initiative Tracker", nullptr, &show_initiative_tracker);
            ImGui::MenuItem("Character Editor", nullptr, &show_character_editor);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Build")) {
            if (ImGui::MenuItem("Build")) {}
            if (ImGui::MenuItem("Clean")) {}
            if (ImGui::MenuItem("Build Config")) {}
            if (ImGui::MenuItem("Toolchain")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Package")) {}
            if (ImGui::MenuItem("Compile Shaders")) {}
            if (ImGui::MenuItem("Asset Pipeline")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Runtime")) {
            if (ImGui::MenuItem("Run")) {}
            if (ImGui::MenuItem("Run with options")) {}
            if (ImGui::MenuItem("Attach Debugger")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Diagnostics")) {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // Dockspace
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f),
                        ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    // Draw windows and widgets
    if (show_asset_manager)    draw_asset_manager();
    if (show_file_explorer)    draw_file_explorer();
    if (show_style_editor)     draw_style_editor();
    if (show_image_viewer)     draw_image_viewer();
    if (show_level_editor)     draw_level_editor();
    if (show_text_editor)      draw_text_editor();
    if (show_imgui_demo)       draw_imgui_demo();
    if (show_console)          draw_console();
    if (show_preview)          draw_preview();
    if (show_output)           draw_output();
    if (compat_modal.open)     draw_compat();

}

static void draw_file_explorer() {
    ImGui::Begin("File Explorer", &show_file_explorer);

    static std::filesystem::path current_dir = std::getenv("HOME");

    // Navbar for explorer
    ImGui::Text("Current: %s", current_dir.string().c_str());

    if (ImGui::Button("Up")) {
        if (current_dir.has_parent_path())
            current_dir = current_dir.parent_path();
    }

    ImGui::Separator();

    // File list
    ImGui::BeginChild("FileList", ImVec2(0, -120), true);

    try {
        for (const auto& entry : std::filesystem::directory_iterator(current_dir)) {
            const auto& path = entry.path();
            std::string label = path.filename().string();

            if (entry.is_directory())
                label = "[D] " + label;

            bool selected =
                explorer_selection.valid &&
                explorer_selection.path == path;

            if (ImGui::Selectable(label.c_str(), selected)) {
                explorer_selection.path = path;
                explorer_selection.valid = true;
            }

            // Double click behavior
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                if (entry.is_directory()) {
                    current_dir = path;
                } else {
                    dispatch_file_action(file_action_type::open, path);
                }
            }
        }
    }
    catch (const std::exception& e) {
        log_error(std::string("Filesystem error: ") + e.what());
    }

    ImGui::EndChild();

    // --- Actions Panel ---
    ImGui::Separator();
    ImGui::Text("Actions");

    ImGui::BeginChild("ActionsPanel", ImVec2(0, 80), true);

    if (explorer_selection.valid) {
        auto actions = resolve_actions(explorer_selection.path);

        for (const auto& action : actions) {
            if (ImGui::Button(action.label.c_str(), ImVec2(-1, 0))) {
                dispatch_file_action(action.type, explorer_selection.path);
            }
        }
    }
    else {
        ImGui::TextDisabled("No file selected");
    }

    ImGui::EndChild();

    ImGui::End();
}

static void draw_style_editor() {
    ImGui::Begin("Dear ImGui Style Editor", &show_style_editor);
    ImGui::ShowStyleEditor();
    ImGui::End();
}

static void draw_level_editor() {
    ImGui::Begin("Level Editor", &show_level_editor);
    ImGui::End();
}

static void draw_asset_manager() {
    //     ImGuiIO& io = ImGui::GetIO();
    //     static float f = 0.0f;


    //     ImGui::Begin("Asset Manager");                          // Create a window called "Hello, world!" and append into it.


    //     ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
    //     ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

    //     ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    //     ImGui::End();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::Begin("Asset Manager");

    // --- Persistent state ---
    static std::vector<::sigil::platform::compat_profile_t> profiles;
    static bool loaded = false;
    static int selected_index = -1;
    static bool shutdown_on_launch = false;
    static bool launch_music_player = false;

    // --- One-time load ---
    if (!loaded) {
        auto ret = ::sigil::platform::probe_compat_profiles(app_info, profiles);
        if (ret.is_failure()) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to load compatibility profiles");
        }
        loaded = true;
    }

    // --- Controls ---
    if (ImGui::Button("Reload Profiles")) {
        profiles.clear();
        selected_index = -1;

        auto ret = ::sigil::platform::probe_compat_profiles(app_info, profiles);
        if (ret.is_failure()) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Reload failed");
        }
    }

    ImGui::SameLine();
    ImGui::Checkbox("Shutdown on Launch", &shutdown_on_launch);
    ImGui::SameLine();
    ImGui::Checkbox("Launch Player", &launch_music_player);

    ImGui::Separator();

    // --- List ---
    if (ImGui::BeginListBox("##compat_profiles", ImVec2(-FLT_MIN, 300))) {

        for (int i = 0; i < (int)profiles.size(); ++i) {
            const auto& profile = profiles[i];

            // Use target path as label (user-facing identifier)
            std::string label = profile.target.string();

            bool is_selected = (selected_index == i);

            if (ImGui::Selectable(label.c_str(), is_selected)) {
                selected_index = i;
            }

            // --- Double-click execution ---
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {

                auto ret = ::sigil::platform::run_compat_profile(app_info, profile);
                if (ret.is_failure()) {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "Execution failed");
                } else {
                    // --- Optional: launch music player ---
                    if (launch_music_player && !profile.extra.empty()) {
                        sigil::platform::proc_exec_unit_t peu;
                        peu.set_exec_mode(sigil::platform::EXEC_DETACH);
                        peu.set_target("vlc");
                        peu.push_argument(profile.extra);
                        sigil::platform::execute(peu);
                    }

                    // --- Optional: shutdown ---
                    if (shutdown_on_launch) {
                        g_shutdown_requested.store(true);
                    }
                }
            }

            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndListBox();
    }

    // --- Footer info ---
    ImGui::Text("Profiles: %zu", profiles.size());
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        1000.0f / io.Framerate, io.Framerate);

    ImGui::End();
}

static void draw_console() {
    ImGui::Begin("Console");

    // Output
    ImGui::BeginChild("ScrollRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false);

    for (const auto& line : console.lines) {
        ImGui::TextUnformatted(line.c_str());
    }

    if (console.scroll_to_bottom) {
        ImGui::SetScrollHereY(1.0f);
        console.scroll_to_bottom = false;
    }

    ImGui::EndChild();

    // Input
    if (console.input_buffer.capacity() == 0)
        console.input_buffer.reserve(256);

    ImGui::PushItemWidth(-1);

    if (ImGui::InputText(
            "##input",
            console.input_buffer.data(),
            console.input_buffer.capacity() + 1,
            ImGuiInputTextFlags_EnterReturnsTrue |
            ImGuiInputTextFlags_CallbackResize,
            input_text_callback,
            (void*)&console.input_buffer))
    {
        std::string cmd = console.input_buffer;

        console.lines.push_back("> " + cmd);

        execute_command(cmd);

        console.input_buffer.clear();
    }

    ImGui::PopItemWidth();

    ImGui::End();
}

static void draw_imgui_demo() {
    ImGui::ShowDemoWindow(&show_imgui_demo);
}

static void draw_text_editor() {
    ImGui::Begin("Text Editor", &show_text_editor);

    if (text_editor.documents.empty()) {
        ImGui::TextDisabled("No file opened");
        ImGui::End();
    } else {
        if (ImGui::BeginTabBar("TextEditorTabs")) {
            for (int i = 0; i < (int)text_editor.documents.size(); ++i) {
                auto& doc = text_editor.documents[i];

                std::string tab_name = doc.path.filename().string();
                if (doc.dirty)
                    tab_name += " *";

                if (ImGui::BeginTabItem(tab_name.c_str(), &doc.open))
                {
                    text_editor.active_index = i;

                    // --- Editor Area ---
                    ImGui::PushItemWidth(-1);

                    if (doc.buffer.capacity() == 0)
                        doc.buffer.reserve(1024);

                    if (ImGui::InputTextMultiline(
                            "##editor",
                            doc.buffer.data(),
                            doc.buffer.capacity() + 1,
                            ImVec2(-1, -ImGui::GetFrameHeightWithSpacing()),
                            ImGuiInputTextFlags_CallbackResize,
                            text_editor_input_callback,
                            (void*)&doc.buffer))
                    {
                        doc.dirty = true;
                    }

                    ImGui::PopItemWidth();

                    // --- Footer Controls ---
                    if (ImGui::Button("Save"))
                    {
                        save_text_editor_document(doc);
                    }

                    ImGui::SameLine();
                    ImGui::TextDisabled("%s", doc.path.string().c_str());

                    ImGui::EndTabItem();
                }
            }

            ImGui::EndTabBar();
        }

        // Cleanup closed tabs
        text_editor.documents.erase(
            std::remove_if(
                text_editor.documents.begin(),
                text_editor.documents.end(),
                [](const text_editor_document_t& d) { return !d.open; }),
            text_editor.documents.end());

        ImGui::End();
    }
}

static void draw_preview() {
    ImGui::Begin("Preview", &show_preview);
    ImGui::End();
}

static void draw_output() {
    ImGui::Begin("Output");

    // Optional controls
    if (ImGui::Button("Clear"))
    {
        log_output.lines.clear();
    }
    ImGui::SameLine();
    ImGui::TextUnformatted("Output Log");

    ImGui::Separator();

    // Scrollable region
    ImGui::BeginChild("OutputScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& line : log_output.lines)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(line.color.v[0], line.color.v[1], line.color.v[2], line.color.v[3]));
        ImGui::TextUnformatted(line.text.c_str());
        ImGui::PopStyleColor();
    }

    if (log_output.scroll_to_bottom)
    {
        ImGui::SetScrollHereY(1.0f);
        log_output.scroll_to_bottom = false;
    }

    ImGui::EndChild();

    ImGui::End();
}

static void draw_compat() {
    ImGui::OpenPopup("Run Executable");

    if (ImGui::BeginPopupModal("Run Executable", &compat_modal.open)) {
        ImGui::Text("Target: %s", compat_modal.target.string().c_str());
        ImGui::Separator();

        // --- Prefix ---
        ImGui::Text("Select Prefix:");
        for (int i = 0; i < (int)compat_modal.prefixes.size(); ++i)
        {
            std::string name = compat_modal.prefixes[i].filename().string();

            if (ImGui::Selectable(name.c_str(), compat_modal.selected_prefix == i))
                compat_modal.selected_prefix = i;
        }

        ImGui::Separator();

        // --- Runtime ---
        ImGui::Text("Select Runtime:");
        for (int i = 0; i < (int)compat_modal.tools.size(); ++i)
        {
            auto& tool = compat_modal.tools[i];

            if (ImGui::Selectable(tool.name.c_str(), compat_modal.selected_tool == i))
                compat_modal.selected_tool = i;
        }

        ImGui::Separator();

        // --- Run ---
        if (ImGui::Button("Run")) {
            if (compat_modal.selected_prefix < 0) {
                log_error("No prefix selected");
            } else if (compat_modal.selected_tool < 0) {
                log_error("No runtime selected");
            } else {
                sigil::platform::compat_profile_t profile;

                profile.target = compat_modal.target;
                profile.prefix = compat_modal.prefixes[compat_modal.selected_prefix];
                profile.runner = compat_modal.tools[compat_modal.selected_tool].path;

                if (!profile.is_valid())
                {
                    log_error("Invalid compat profile");
                }
                else
                {
                    auto y = sigil::platform::run_compat_profile(app_info, profile);

                    if (y.is_failure())
                    {
                        log_error("Execution failed");
                    }
                    else
                    {
                        log_info("Execution started: " + profile.target.string());
                    }

                    compat_modal.open = false;
                }
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            compat_modal.open = false;
        }

        ImGui::EndPopup();
    }
}

static void draw_image_viewer() {
    ImGui::Begin("Image Viewer");
    ImGui::End();
}
