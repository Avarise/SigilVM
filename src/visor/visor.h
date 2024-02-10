#pragma once
/*
    Graphics modules for SigilVM
    Visor uses Vulkan + GLFW + Dear ImGui to achieve graphics capabilities.
    Based on Dear ImGui provided renderer
*/
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <vector>
#include <cstdio>

#include "../core/runtime.h"
#include "../core/modules.h"
#include "../imgui/imgui.h"

#   if defined(IMGUI_IMPL_VULKAN_NO_PROTOTYPES) && !defined(VK_NO_PROTOTYPES)
#       define VK_NO_PROTOTYPES
#   endif

#   if defined(VK_VERSION_1_3) || defined(VK_KHR_dynamic_rendering)
#       define VISOR_VK_HAS_DYNAMIC_RENDERING
#   endif

namespace sigil {
    namespace visor {
        struct vk_init_info_t {
            VkInstance                      Instance;
            VkPhysicalDevice                PhysicalDevice;
            VkDevice                        Device;
            uint32_t                        QueueFamily;
            VkQueue                         Queue;
            VkDescriptorPool                DescriptorPool;               // See requirements in note above
            VkRenderPass                    RenderPass;                   // Ignored if using dynamic rendering
            uint32_t                        MinImageCount;                // >= 2
            uint32_t                        ImageCount;                   // >= MinImageCount
            VkSampleCountFlagBits           MSAASamples;                  // 0 defaults to VK_SAMPLE_COUNT_1_BIT

            // (Optional)
            VkPipelineCache                 PipelineCache;
            uint32_t                        Subpass;

            // (Optional) Dynamic Rendering
            // Need to explicitly enable VK_KHR_dynamic_rendering extension to use this, even for Vulkan 1.3.
            bool                            UseDynamicRendering;
        #ifdef VISOR_VK_HAS_DYNAMIC_RENDERING
            VkPipelineRenderingCreateInfoKHR PipelineRenderingCreateInfo;
        #endif

            // (Optional) Allocation, Debugging
            const VkAllocationCallbacks*    Allocator;
            void                            (*CheckVkResultFn)(VkResult err);
            VkDeviceSize                    MinAllocationSize;      // Minimum allocation size. Set to 1024*1024 to satisfy zealous best practices validation layer and waste a little memory.
        };

        struct vk_framedata_t {
            VkCommandPool       CommandPool;
            VkCommandBuffer     CommandBuffer;
            VkFence             Fence;
            VkImage             Backbuffer;
            VkImageView         BackbufferView;
            VkFramebuffer       Framebuffer;
        };
        
        // Called by user code
        bool vk_init(vk_init_info_t* info);
        void ImGui_ImplVulkan_Shutdown();
        void ImGui_ImplVulkan_NewFrame();
        void ImGui_ImplVulkan_RenderDrawData(ImDrawData* draw_data, VkCommandBuffer command_buffer, VkPipeline pipeline = VK_NULL_HANDLE);
        bool ImGui_ImplVulkan_CreateFontsTexture();
        void ImGui_ImplVulkan_DestroyFontsTexture();
        void ImGui_ImplVulkan_SetMinImageCount(uint32_t min_image_count); // To override MinImageCount after initialization (e.g. if swap chain is recreated)

        // Register a texture (VkDescriptorSet == ImTextureID)
        // FIXME: This is experimental in the sense that we are unsure how to best design/tackle this problem
        // Please post to https://github.com/ocornut/imgui/pull/914 if you have suggestions.
        VkDescriptorSet ImGui_ImplVulkan_AddTexture(VkSampler sampler, VkImageView image_view, VkImageLayout image_layout);
        void            ImGui_ImplVulkan_RemoveTexture(VkDescriptorSet descriptor_set);

        // Optional: load Vulkan functions with a custom function loader
        // This is only useful with IMGUI_IMPL_VULKAN_NO_PROTOTYPES / VK_NO_PROTOTYPES
        bool         ImGui_ImplVulkan_LoadFunctions(PFN_vkVoidFunction(*loader_func)(const char* function_name, void* user_data), void* user_data = nullptr);

        struct vk_frame_semaphores_t {
            VkSemaphore         ImageAcquiredSemaphore;
            VkSemaphore         RenderCompleteSemaphore;
        };

        struct vk_queue {
            VkQueue queue;
            uint32_t family;
        };

        struct vk_window_t {
            int min_image_count;
            bool rebuild_swap_chain;
            int                 Width;
            int                 Height;
            VkSwapchainKHR      Swapchain;
            VkSurfaceKHR        Surface;
            VkSurfaceFormatKHR  SurfaceFormat;
            VkPresentModeKHR    PresentMode;
            VkRenderPass        RenderPass;
            VkPipeline          Pipeline;               // The window pipeline may uses a different VkRenderPass than the one passed in ImGui_ImplVulkan_InitInfo
            bool                UseDynamicRendering;
            bool                ClearEnable;
            VkClearValue        ClearValue;
            uint32_t            FrameIndex;             // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
            uint32_t            ImageCount;             // Number of simultaneous in-flight frames (returned by vkGetSwapchainImagesKHR, usually derived from min_image_count)
            uint32_t            SemaphoreCount;         // Number of simultaneous in-flight frames + 1, to be able to use it in vkAcquireNextImageKHR
            uint32_t            SemaphoreIndex;         // Current set of swapchain wait semaphores we're using (needs to be distinct from per frame data)
            sigil::visor::vk_framedata_t*            Frames;
            sigil::visor::vk_frame_semaphores_t*  FrameSemaphores;
            GLFWwindow *glfw_window;

            vk_window_t() {
                memset((void*)this, 0, sizeof(*this));
                PresentMode = (VkPresentModeKHR)~0;     // Ensure we get an error if user doesn't set this.
                ClearEnable = true;
            }
        };

        struct module_data_t {
            VkInstance vk_inst;
            VkDevice vk_dev; // Logical device for Vulkan
            uint32_t num_phys; // Number of physical devices found
            std::vector<VkPhysicalDevice> vk_phy_all; // Vector of all physical devices found
            VkPhysicalDevice vk_phy_active; // Chosen active GPU
            VkPhysicalDevice vk_phy_aux; // Chosen aux GPU
            //std::vector<vk_queue> vk_queues;
            vk_queue vk_main_q;
            vk_queue vk_graphics_q;
            vk_queue vk_compute_q;
            VkAllocationCallbacks* vk_allocators;
            VkDebugReportCallbackEXT vk_dbg_callback_ext;
            VkPipelineCache vk_pipeline_cache;
            VkDescriptorPool vk_descriptor_pool;
            vk_window_t vk_window;
            int min_image_count = 2;
            bool rebuild_swap_chain = false;
            bool vsync = true;
            bool fullscreen = false;
            //VkSurfaceKHR vk_surface;
            //std::vector<vk_window_t> vk_windows;
        };

        extern sigil::modules::module_t *module;
        extern module_data_t *data;

        int set_vk_present_mode(bool vsync);
        int destroy_vk_window(VkInstance instance, VkDevice device, vk_window_t *window, const VkAllocationCallbacks* allocator);
        const char* vk_result_to_cstr(VkResult err);
        void check_vk_result(VkResult err);
        bool is_vk_ext_available(const std::vector<VkExtensionProperties> &properties, const char* extension);
        int vk_setup(std::vector<const char*> instance_extensions);
        int vk_inst_create(std::vector<const char*> vk_inst_ext);
        void vk_auto_select_gpu();
        int vk_setup_descriptor_pool();
        int vk_setup_queues() ;
        int vk_setup_window(vk_window_t* vk_window, VkSurfaceKHR surface, int width, int height);
        inline void glfw_error_callback(int error, const char* description) {
            fprintf(stderr, "glfw: error %d: %s\n", error, description);
        }

        int create_main_window();


        int init(void *ctx);
        int deinit(void *ctx);
        int import();
    }
}


IMGUI_IMPL_API void         ImGui_ImplVulkan_Shutdown();
IMGUI_IMPL_API void         ImGui_ImplVulkan_NewFrame();
IMGUI_IMPL_API void         ImGui_ImplVulkan_RenderDrawData(ImDrawData* draw_data, VkCommandBuffer command_buffer, VkPipeline pipeline = VK_NULL_HANDLE);
IMGUI_IMPL_API bool         ImGui_ImplVulkan_CreateFontsTexture();
IMGUI_IMPL_API void         ImGui_ImplVulkan_DestroyFontsTexture();
IMGUI_IMPL_API void         ImGui_ImplVulkan_SetMinImageCount(uint32_t min_image_count); // To override MinImageCount after initialization (e.g. if swap chain is recreated)

// Register a texture (VkDescriptorSet == ImTextureID)
// FIXME: This is experimental in the sense that we are unsure how to best design/tackle this problem
// Please post to https://github.com/ocornut/imgui/pull/914 if you have suggestions.
IMGUI_IMPL_API VkDescriptorSet ImGui_ImplVulkan_AddTexture(VkSampler sampler, VkImageView image_view, VkImageLayout image_layout);
IMGUI_IMPL_API void            ImGui_ImplVulkan_RemoveTexture(VkDescriptorSet descriptor_set);

// Optional: load Vulkan functions with a custom function loader
// This is only useful with IMGUI_IMPL_VULKAN_NO_PROTOTYPES / VK_NO_PROTOTYPES
IMGUI_IMPL_API bool         ImGui_ImplVulkan_LoadFunctions(PFN_vkVoidFunction(*loader_func)(const char* function_name, void* user_data), void* user_data = nullptr);
// Helpers
IMGUI_IMPL_API void                 ImGui_ImplVulkanH_CreateOrResizeWindow(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device, sigil::visor::vk_window_t* wnd, uint32_t queue_family, const VkAllocationCallbacks* allocator, int w, int h, uint32_t min_image_count);
IMGUI_IMPL_API void                 ImGui_ImplVulkanH_DestroyWindow(VkInstance instance, VkDevice device, sigil::visor::vk_window_t* wnd, const VkAllocationCallbacks* allocator);
IMGUI_IMPL_API VkSurfaceFormatKHR   ImGui_ImplVulkanH_SelectSurfaceFormat(VkPhysicalDevice physical_device, VkSurfaceKHR surface, const VkFormat* request_formats, int request_formats_count, VkColorSpaceKHR request_color_space);
IMGUI_IMPL_API VkPresentModeKHR     ImGui_ImplVulkanH_SelectPresentMode(VkPhysicalDevice physical_device, VkSurfaceKHR surface, const VkPresentModeKHR* request_modes, int request_modes_count);
IMGUI_IMPL_API int                  ImGui_ImplVulkanH_GetMinImageCountFromPresentMode(VkPresentModeKHR present_mode);
