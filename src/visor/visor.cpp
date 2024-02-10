#include <cassert>
#include <cstddef>
#include <stdio.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "../../assets/shaders/cshader-example-frag.h"
#include "../../assets/shaders/cshader-example-vert.h"
#include "../../assets/shaders/cshader-vulkan-tutorial-frag.h"
#include "../../assets/shaders/cshader-vulkan-tutorial-vert.h"
#include "../imgui/imgui.h"
#include "visor.h"
#include "../core/env.h"
#ifndef IM_MAX
#define IM_MAX(A, B)    (((A) >= (B)) ? (A) : (B))
#endif
sigil::modules::module_t *sigil::visor::module = NULL;
// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127) // condition expression is constant
#endif

sigil::visor::module_data_t *sigil::visor::data = NULL;

// Forward Declarations
struct ImGui_ImplVulkan_FrameRenderBuffers;
struct ImGui_ImplVulkan_WindowRenderBuffers;
bool ImGui_ImplVulkan_CreateDeviceObjects();
void ImGui_ImplVulkan_DestroyDeviceObjects();
void ImGui_ImplVulkan_DestroyFrameRenderBuffers(VkDevice device, ImGui_ImplVulkan_FrameRenderBuffers* buffers, const VkAllocationCallbacks* allocator);
void ImGui_ImplVulkan_DestroyWindowRenderBuffers(VkDevice device, ImGui_ImplVulkan_WindowRenderBuffers* buffers, const VkAllocationCallbacks* allocator);
void ImGui_ImplVulkanH_DestroyFrame(VkDevice device, sigil::visor::vk_framedata_t* fd, const VkAllocationCallbacks* allocator);
void ImGui_ImplVulkanH_DestroyFrameSemaphores(VkDevice device, sigil::visor::vk_frame_semaphores_t* fsd, const VkAllocationCallbacks* allocator);
void ImGui_ImplVulkanH_CreateWindowSwapChain(VkPhysicalDevice physical_device, VkDevice device, sigil::visor::vk_window_t* wd, const VkAllocationCallbacks* allocator, int w, int h, uint32_t min_image_count);
void ImGui_ImplVulkanH_CreateWindowCommandBuffers(VkPhysicalDevice physical_device, VkDevice device, sigil::visor::vk_window_t* wd, uint32_t queue_family, const VkAllocationCallbacks* allocator);

// Vulkan prototypes for use with custom loaders
// (see description of IMGUI_IMPL_VULKAN_NO_PROTOTYPES in imgui_impl_vulkan.h
#ifdef VK_NO_PROTOTYPES
static bool g_FunctionsLoaded = false;
#else
static bool g_FunctionsLoaded = true;
#endif
#ifdef VK_NO_PROTOTYPES
#define IMGUI_VULKAN_FUNC_MAP(IMGUI_VULKAN_FUNC_MAP_MACRO) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkAllocateCommandBuffers) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkAllocateDescriptorSets) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkAllocateMemory) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkBeginCommandBuffer) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkBindBufferMemory) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkBindImageMemory) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCmdBindDescriptorSets) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCmdBindIndexBuffer) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCmdBindPipeline) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCmdBindVertexBuffers) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCmdCopyBufferToImage) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCmdDrawIndexed) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCmdPipelineBarrier) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCmdPushConstants) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCmdSetScissor) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCmdSetViewport) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCreateBuffer) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCreateCommandPool) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCreateDescriptorSetLayout) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCreateFence) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCreateFramebuffer) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCreateGraphicsPipelines) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCreateImage) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCreateImageView) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCreatePipelineLayout) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCreateRenderPass) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCreateSampler) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCreateSemaphore) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCreateShaderModule) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkCreateSwapchainKHR) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkDestroyBuffer) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkDestroyCommandPool) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkDestroyDescriptorSetLayout) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkDestroyFence) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkDestroyFramebuffer) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkDestroyImage) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkDestroyImageView) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkDestroyPipeline) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkDestroyPipelineLayout) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkDestroyRenderPass) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkDestroySampler) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkDestroySemaphore) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkDestroyShaderModule) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkDestroySurfaceKHR) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkDestroySwapchainKHR) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkDeviceWaitIdle) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkEndCommandBuffer) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkFlushMappedMemoryRanges) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkFreeCommandBuffers) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkFreeDescriptorSets) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkFreeMemory) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkGetBufferMemoryRequirements) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkGetImageMemoryRequirements) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkGetPhysicalDeviceMemoryProperties) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkGetPhysicalDeviceSurfaceCapabilitiesKHR) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkGetPhysicalDeviceSurfaceFormatsKHR) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkGetPhysicalDeviceSurfacePresentModesKHR) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkGetSwapchainImagesKHR) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkMapMemory) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkQueueSubmit) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkQueueWaitIdle) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkResetCommandPool) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkUnmapMemory) \
    IMGUI_VULKAN_FUNC_MAP_MACRO(vkUpdateDescriptorSets)

// Define function pointers
#define IMGUI_VULKAN_FUNC_DEF(func) static PFN_##func func;
IMGUI_VULKAN_FUNC_MAP(IMGUI_VULKAN_FUNC_DEF)
#undef IMGUI_VULKAN_FUNC_DEF
#endif // VK_NO_PROTOTYPES

#ifdef IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
static PFN_vkCmdBeginRenderingKHR   ImGuiImplVulkanFuncs_vkCmdBeginRenderingKHR;
static PFN_vkCmdEndRenderingKHR     ImGuiImplVulkanFuncs_vkCmdEndRenderingKHR;
#endif

// Reusable buffers used for rendering 1 current in-flight frame, for ImGui_ImplVulkan_RenderDrawData()
// [Please zero-clear before use!]
struct ImGui_ImplVulkan_FrameRenderBuffers
{
    VkDeviceMemory      VertexBufferMemory;
    VkDeviceMemory      IndexBufferMemory;
    VkDeviceSize        VertexBufferSize;
    VkDeviceSize        IndexBufferSize;
    VkBuffer            VertexBuffer;
    VkBuffer            IndexBuffer;
};

// Each viewport will hold 1 sigil::visor::vk_window_tRenderBuffers
// [Please zero-clear before use!]
struct ImGui_ImplVulkan_WindowRenderBuffers
{
    uint32_t            Index;
    uint32_t            Count;
    ImGui_ImplVulkan_FrameRenderBuffers* FrameRenderBuffers;
};

// Vulkan data
struct ImGui_ImplVulkan_Data
{
    sigil::visor::vk_init_info_t   VulkanInitInfo;
    VkDeviceSize                BufferMemoryAlignment;
    VkPipelineCreateFlags       PipelineCreateFlags;
    VkDescriptorSetLayout       DescriptorSetLayout;
    VkPipelineLayout            PipelineLayout;
    VkPipeline                  Pipeline;
    VkShaderModule              ShaderModuleVert;
    VkShaderModule              ShaderModuleFrag;
    VkShaderModule              some_shader;

    // Font data
    VkSampler                   FontSampler;
    VkDeviceMemory              FontMemory;
    VkImage                     FontImage;
    VkImageView                 FontView;
    VkDescriptorSet             FontDescriptorSet;
    VkCommandPool               FontCommandPool;
    VkCommandBuffer             FontCommandBuffer;

    // Render buffers for main window
    ImGui_ImplVulkan_WindowRenderBuffers MainWindowRenderBuffers;

    ImGui_ImplVulkan_Data()
    {
        memset((void*)this, 0, sizeof(*this));
        BufferMemoryAlignment = 256;
    }
};

//-----------------------------------------------------------------------------
// FUNCTIONS
//-----------------------------------------------------------------------------

// Backend data stored in io.BackendRendererUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
// FIXME: multi-context support is not tested and probably dysfunctional in this backend.
static ImGui_ImplVulkan_Data* ImGui_ImplVulkan_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplVulkan_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
}

static uint32_t ImGui_ImplVulkan_MemoryType(VkMemoryPropertyFlags properties, uint32_t type_bits)
{
    ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
    sigil::visor::vk_init_info_t* v = &bd->VulkanInitInfo;
    VkPhysicalDeviceMemoryProperties prop;
    vkGetPhysicalDeviceMemoryProperties(v->PhysicalDevice, &prop);
    for (uint32_t i = 0; i < prop.memoryTypeCount; i++)
        if ((prop.memoryTypes[i].propertyFlags & properties) == properties && type_bits & (1 << i))
            return i;
    return 0xFFFFFFFF; // Unable to find memoryType
}

const char* sigil::visor::vk_result_to_cstr(VkResult err) {
    if (err == VK_SUCCESS) return "VK_SUCCESS";
    if (err == VK_NOT_READY) return "VK_NOT_READY";
    if (err == VK_TIMEOUT) return "VK_TIMEOUT";
    if (err == VK_EVENT_SET) return "VK_EVENT_SET";
    if (err == VK_EVENT_RESET) return "VK_EVENT_RESET";
    if (err == VK_INCOMPLETE) return "VK_INCOMPLETE";
    if (err == VK_ERROR_OUT_OF_HOST_MEMORY) return "VK_ERROR_OUT_OF_HOST_MEMORY";
    if (err == VK_ERROR_OUT_OF_DEVICE_MEMORY) return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    if (err == VK_ERROR_INITIALIZATION_FAILED) return "VK_ERROR_INITIALIZATION_FAILED";
    if (err == VK_ERROR_DEVICE_LOST) return "VK_ERROR_DEVICE_LOST";
    if (err == VK_ERROR_MEMORY_MAP_FAILED) return "VK_ERROR_MEMORY_MAP_FAILED";
    if (err == VK_ERROR_LAYER_NOT_PRESENT) return "VK_ERROR_LAYER_NOT_PRESENT";
    if (err == VK_ERROR_EXTENSION_NOT_PRESENT) return "VK_ERROR_EXTENSION_NOT_PRESENT";
    if (err == VK_SUCCESS) return "VK_SUCCESS";
    if (err == VK_SUCCESS) return "VK_SUCCESS";
    if (err == VK_SUCCESS) return "VK_SUCCESS";
    if (err == VK_SUCCESS) return "VK_SUCCESS";
    if (err == VK_SUCCESS) return "VK_SUCCESS";
    if (err == VK_SUCCESS) return "VK_SUCCESS";
    if (err == VK_SUCCESS) return "VK_SUCCESS";
    if (err == VK_SUCCESS) return "VK_SUCCESS";
    return NULL;
}

void sigil::visor::check_vk_result(VkResult err) {
    if (err == 0) return;
    fprintf(stderr, "visor: Vulkan response %d (%s)\n", err, sigil::visor::vk_result_to_cstr(err));
    if (err < 0) abort();
}

// Same as IM_MEMALIGN(). 'alignment' must be a power of two.
static inline VkDeviceSize AlignBufferSize(VkDeviceSize size, VkDeviceSize alignment)
{
    return (size + alignment - 1) & ~(alignment - 1);
}

static void CreateOrResizeBuffer(VkBuffer& buffer, VkDeviceMemory& buffer_memory, VkDeviceSize& buffer_size, size_t new_size, VkBufferUsageFlagBits usage)
{
    ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
    sigil::visor::vk_init_info_t* v = &bd->VulkanInitInfo;
    VkResult err;
    if (buffer != VK_NULL_HANDLE)
        vkDestroyBuffer(v->Device, buffer, v->Allocator);
    if (buffer_memory != VK_NULL_HANDLE)
        vkFreeMemory(v->Device, buffer_memory, v->Allocator);

    VkDeviceSize buffer_size_aligned = AlignBufferSize(IM_MAX(v->MinAllocationSize, new_size), bd->BufferMemoryAlignment);
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = buffer_size_aligned;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    err = vkCreateBuffer(v->Device, &buffer_info, v->Allocator, &buffer);
    sigil::visor::check_vk_result(err);

    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(v->Device, buffer, &req);
    bd->BufferMemoryAlignment = (bd->BufferMemoryAlignment > req.alignment) ? bd->BufferMemoryAlignment : req.alignment;
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = req.size;
    alloc_info.memoryTypeIndex = ImGui_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
    err = vkAllocateMemory(v->Device, &alloc_info, v->Allocator, &buffer_memory);
    sigil::visor::check_vk_result(err);

    err = vkBindBufferMemory(v->Device, buffer, buffer_memory, 0);
    sigil::visor::check_vk_result(err);
    buffer_size = buffer_size_aligned;
}

static void ImGui_ImplVulkan_SetupRenderState(ImDrawData* draw_data, VkPipeline pipeline, VkCommandBuffer command_buffer, ImGui_ImplVulkan_FrameRenderBuffers* rb, int fb_width, int fb_height)
{
    ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();

    // Bind pipeline:
    {
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    // Bind Vertex And Index Buffer:
    if (draw_data->TotalVtxCount > 0)
    {
        VkBuffer vertex_buffers[1] = { rb->VertexBuffer };
        VkDeviceSize vertex_offset[1] = { 0 };
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, vertex_offset);
        vkCmdBindIndexBuffer(command_buffer, rb->IndexBuffer, 0, sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
    }

    // Setup viewport:
    {
        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = (float)fb_width;
        viewport.height = (float)fb_height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    }

    // Setup scale and translation:
    // Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    {
        float scale[2];
        scale[0] = 2.0f / draw_data->DisplaySize.x;
        scale[1] = 2.0f / draw_data->DisplaySize.y;
        float translate[2];
        translate[0] = -1.0f - draw_data->DisplayPos.x * scale[0];
        translate[1] = -1.0f - draw_data->DisplayPos.y * scale[1];
        vkCmdPushConstants(command_buffer, bd->PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 0, sizeof(float) * 2, scale);
        vkCmdPushConstants(command_buffer, bd->PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);
    }
}

// Render function
void ImGui_ImplVulkan_RenderDrawData(ImDrawData* draw_data, VkCommandBuffer command_buffer, VkPipeline pipeline)
{
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;

    ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
    sigil::visor::vk_init_info_t* v = &bd->VulkanInitInfo;
    if (pipeline == VK_NULL_HANDLE)
        pipeline = bd->Pipeline;

    // Allocate array to store enough vertex/index buffers
    ImGui_ImplVulkan_WindowRenderBuffers* wrb = &bd->MainWindowRenderBuffers;
    if (wrb->FrameRenderBuffers == nullptr)
    {
        wrb->Index = 0;
        wrb->Count = v->ImageCount;
        wrb->FrameRenderBuffers = (ImGui_ImplVulkan_FrameRenderBuffers*)IM_ALLOC(sizeof(ImGui_ImplVulkan_FrameRenderBuffers) * wrb->Count);
        memset(wrb->FrameRenderBuffers, 0, sizeof(ImGui_ImplVulkan_FrameRenderBuffers) * wrb->Count);
    }
    IM_ASSERT(wrb->Count == v->ImageCount);
    wrb->Index = (wrb->Index + 1) % wrb->Count;
    ImGui_ImplVulkan_FrameRenderBuffers* rb = &wrb->FrameRenderBuffers[wrb->Index];

    if (draw_data->TotalVtxCount > 0)
    {
        // Create or resize the vertex/index buffers
        size_t vertex_size = AlignBufferSize(draw_data->TotalVtxCount * sizeof(ImDrawVert), bd->BufferMemoryAlignment);
        size_t index_size = AlignBufferSize(draw_data->TotalIdxCount * sizeof(ImDrawIdx), bd->BufferMemoryAlignment);
        if (rb->VertexBuffer == VK_NULL_HANDLE || rb->VertexBufferSize < vertex_size)
            CreateOrResizeBuffer(rb->VertexBuffer, rb->VertexBufferMemory, rb->VertexBufferSize, vertex_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        if (rb->IndexBuffer == VK_NULL_HANDLE || rb->IndexBufferSize < index_size)
            CreateOrResizeBuffer(rb->IndexBuffer, rb->IndexBufferMemory, rb->IndexBufferSize, index_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        // Upload vertex/index data into a single contiguous GPU buffer
        ImDrawVert* vtx_dst = nullptr;
        ImDrawIdx* idx_dst = nullptr;
        VkResult err = vkMapMemory(v->Device, rb->VertexBufferMemory, 0, vertex_size, 0, (void**)&vtx_dst);
        sigil::visor::check_vk_result(err);
        err = vkMapMemory(v->Device, rb->IndexBufferMemory, 0, index_size, 0, (void**)&idx_dst);
        sigil::visor::check_vk_result(err);
        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtx_dst += cmd_list->VtxBuffer.Size;
            idx_dst += cmd_list->IdxBuffer.Size;
        }
        VkMappedMemoryRange range[2] = {};
        range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range[0].memory = rb->VertexBufferMemory;
        range[0].size = VK_WHOLE_SIZE;
        range[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range[1].memory = rb->IndexBufferMemory;
        range[1].size = VK_WHOLE_SIZE;
        err = vkFlushMappedMemoryRanges(v->Device, 2, range);
        sigil::visor::check_vk_result(err);
        vkUnmapMemory(v->Device, rb->VertexBufferMemory);
        vkUnmapMemory(v->Device, rb->IndexBufferMemory);
    }

    // Setup desired Vulkan state
    ImGui_ImplVulkan_SetupRenderState(draw_data, pipeline, command_buffer, rb, fb_width, fb_height);

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_vtx_offset = 0;
    int global_idx_offset = 0;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != nullptr)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    ImGui_ImplVulkan_SetupRenderState(draw_data, pipeline, command_buffer, rb, fb_width, fb_height);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

                // Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
                if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
                if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
                if (clip_max.x > fb_width) { clip_max.x = (float)fb_width; }
                if (clip_max.y > fb_height) { clip_max.y = (float)fb_height; }
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                // Apply scissor/clipping rectangle
                VkRect2D scissor;
                scissor.offset.x = (int32_t)(clip_min.x);
                scissor.offset.y = (int32_t)(clip_min.y);
                scissor.extent.width = (uint32_t)(clip_max.x - clip_min.x);
                scissor.extent.height = (uint32_t)(clip_max.y - clip_min.y);
                vkCmdSetScissor(command_buffer, 0, 1, &scissor);

                // Bind DescriptorSet with font or user texture
                VkDescriptorSet desc_set[1] = { (VkDescriptorSet)pcmd->TextureId };
                if (sizeof(ImTextureID) < sizeof(ImU64))
                {
                    // We don't support texture switches if ImTextureID hasn't been redefined to be 64-bit. Do a flaky check that other textures haven't been used.
                    IM_ASSERT(pcmd->TextureId == (ImTextureID)bd->FontDescriptorSet);
                    desc_set[0] = bd->FontDescriptorSet;
                }
                vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, bd->PipelineLayout, 0, 1, desc_set, 0, nullptr);

                // Draw
                vkCmdDrawIndexed(command_buffer, pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }

    // Note: at this point both vkCmdSetViewport() and vkCmdSetScissor() have been called.
    // Our last values will leak into user/application rendering IF:
    // - Your app uses a pipeline with VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR dynamic state
    // - And you forgot to call vkCmdSetViewport() and vkCmdSetScissor() yourself to explicitly set that state.
    // If you use VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR you are responsible for setting the values before rendering.
    // In theory we should aim to backup/restore those values but I am not sure this is possible.
    // We perform a call to vkCmdSetScissor() to set back a full viewport which is likely to fix things for 99% users but technically this is not perfect. (See github #4644)
    VkRect2D scissor = { { 0, 0 }, { (uint32_t)fb_width, (uint32_t)fb_height } };
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

bool ImGui_ImplVulkan_CreateFontsTexture()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
    sigil::visor::vk_init_info_t* v = &bd->VulkanInitInfo;
    VkResult err;

    // Destroy existing texture (if any)
    if (bd->FontView || bd->FontImage || bd->FontMemory || bd->FontDescriptorSet)
    {
        vkQueueWaitIdle(v->Queue);
        ImGui_ImplVulkan_DestroyFontsTexture();
    }

    // Create command pool/buffer
    if (bd->FontCommandPool == VK_NULL_HANDLE)
    {
        VkCommandPoolCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.flags = 0;
        info.queueFamilyIndex = v->QueueFamily;
        vkCreateCommandPool(v->Device, &info, v->Allocator, &bd->FontCommandPool);
    }
    if (bd->FontCommandBuffer == VK_NULL_HANDLE)
    {
        VkCommandBufferAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.commandPool = bd->FontCommandPool;
        info.commandBufferCount = 1;
        err = vkAllocateCommandBuffers(v->Device, &info, &bd->FontCommandBuffer);
        sigil::visor::check_vk_result(err);
    }

    // Start command buffer
    {
        err = vkResetCommandPool(v->Device, bd->FontCommandPool, 0);
        sigil::visor::check_vk_result(err);
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(bd->FontCommandBuffer, &begin_info);
        sigil::visor::check_vk_result(err);
    }

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    size_t upload_size = width * height * 4 * sizeof(char);

    // Create the Image:
    {
        VkImageCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = VK_FORMAT_R8G8B8A8_UNORM;
        info.extent.width = width;
        info.extent.height = height;
        info.extent.depth = 1;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        err = vkCreateImage(v->Device, &info, v->Allocator, &bd->FontImage);
        sigil::visor::check_vk_result(err);
        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(v->Device, bd->FontImage, &req);
        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = IM_MAX(v->MinAllocationSize, req.size);
        alloc_info.memoryTypeIndex = ImGui_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, req.memoryTypeBits);
        err = vkAllocateMemory(v->Device, &alloc_info, v->Allocator, &bd->FontMemory);
        sigil::visor::check_vk_result(err);
        err = vkBindImageMemory(v->Device, bd->FontImage, bd->FontMemory, 0);
        sigil::visor::check_vk_result(err);
    }

    // Create the Image View:
    {
        VkImageViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image = bd->FontImage;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = VK_FORMAT_R8G8B8A8_UNORM;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.layerCount = 1;
        err = vkCreateImageView(v->Device, &info, v->Allocator, &bd->FontView);
        sigil::visor::check_vk_result(err);
    }

    // Create the Descriptor Set:
    bd->FontDescriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(bd->FontSampler, bd->FontView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Create the Upload Buffer:
    VkDeviceMemory upload_buffer_memory;
    VkBuffer upload_buffer;
    {
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = upload_size;
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        err = vkCreateBuffer(v->Device, &buffer_info, v->Allocator, &upload_buffer);
        sigil::visor::check_vk_result(err);
        VkMemoryRequirements req;
        vkGetBufferMemoryRequirements(v->Device, upload_buffer, &req);
        bd->BufferMemoryAlignment = (bd->BufferMemoryAlignment > req.alignment) ? bd->BufferMemoryAlignment : req.alignment;
        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = IM_MAX(v->MinAllocationSize, req.size);
        alloc_info.memoryTypeIndex = ImGui_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
        err = vkAllocateMemory(v->Device, &alloc_info, v->Allocator, &upload_buffer_memory);
        sigil::visor::check_vk_result(err);
        err = vkBindBufferMemory(v->Device, upload_buffer, upload_buffer_memory, 0);
        sigil::visor::check_vk_result(err);
    }

    // Upload to Buffer:
    {
        char* map = nullptr;
        err = vkMapMemory(v->Device, upload_buffer_memory, 0, upload_size, 0, (void**)(&map));
        sigil::visor::check_vk_result(err);
        memcpy(map, pixels, upload_size);
        VkMappedMemoryRange range[1] = {};
        range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range[0].memory = upload_buffer_memory;
        range[0].size = upload_size;
        err = vkFlushMappedMemoryRanges(v->Device, 1, range);
        sigil::visor::check_vk_result(err);
        vkUnmapMemory(v->Device, upload_buffer_memory);
    }

    // Copy to Image:
    {
        VkImageMemoryBarrier copy_barrier[1] = {};
        copy_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        copy_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        copy_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        copy_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copy_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier[0].image = bd->FontImage;
        copy_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_barrier[0].subresourceRange.levelCount = 1;
        copy_barrier[0].subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(bd->FontCommandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, copy_barrier);

        VkBufferImageCopy region = {};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width = width;
        region.imageExtent.height = height;
        region.imageExtent.depth = 1;
        vkCmdCopyBufferToImage(bd->FontCommandBuffer, upload_buffer, bd->FontImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        VkImageMemoryBarrier use_barrier[1] = {};
        use_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        use_barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        use_barrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        use_barrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        use_barrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        use_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        use_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        use_barrier[0].image = bd->FontImage;
        use_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        use_barrier[0].subresourceRange.levelCount = 1;
        use_barrier[0].subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(bd->FontCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, use_barrier);
    }

    // Store our identifier
    io.Fonts->SetTexID((ImTextureID)bd->FontDescriptorSet);

    // End command buffer
    VkSubmitInfo end_info = {};
    end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers = &bd->FontCommandBuffer;
    err = vkEndCommandBuffer(bd->FontCommandBuffer);
    sigil::visor::check_vk_result(err);
    err = vkQueueSubmit(v->Queue, 1, &end_info, VK_NULL_HANDLE);
    sigil::visor::check_vk_result(err);

    err = vkQueueWaitIdle(v->Queue);
    sigil::visor::check_vk_result(err);

    vkDestroyBuffer(v->Device, upload_buffer, v->Allocator);
    vkFreeMemory(v->Device, upload_buffer_memory, v->Allocator);

    return true;
}

// You probably never need to call this, as it is called by ImGui_ImplVulkan_CreateFontsTexture() and ImGui_ImplVulkan_Shutdown().
void ImGui_ImplVulkan_DestroyFontsTexture()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
    sigil::visor::vk_init_info_t* v = &bd->VulkanInitInfo;

    if (bd->FontDescriptorSet)
    {
        ImGui_ImplVulkan_RemoveTexture(bd->FontDescriptorSet);
        bd->FontDescriptorSet = VK_NULL_HANDLE;
        io.Fonts->SetTexID(0);
    }

    if (bd->FontView)   { vkDestroyImageView(v->Device, bd->FontView, v->Allocator); bd->FontView = VK_NULL_HANDLE; }
    if (bd->FontImage)  { vkDestroyImage(v->Device, bd->FontImage, v->Allocator); bd->FontImage = VK_NULL_HANDLE; }
    if (bd->FontMemory) { vkFreeMemory(v->Device, bd->FontMemory, v->Allocator); bd->FontMemory = VK_NULL_HANDLE; }
}

static void ImGui_ImplVulkan_CreateShaderModules(VkDevice device, const VkAllocationCallbacks* allocator)
{
    // Create the shader modules
    ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
    if (bd->ShaderModuleVert == VK_NULL_HANDLE)
    {
        VkShaderModuleCreateInfo vert_info = {};
        vert_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vert_info.codeSize = sizeof(__glsl_shader_vert_spv);
        vert_info.pCode = (uint32_t*)__glsl_shader_vert_spv;
        //vert_info.codeSize = sizeof(__sigil_vk_tut_vert);
        //vert_info.pCode = (uint32_t*)__sigil_vk_tut_vert;
        VkResult err = vkCreateShaderModule(device, &vert_info, allocator, &bd->ShaderModuleVert);
        sigil::visor::check_vk_result(err);
    }
    if (bd->ShaderModuleFrag == VK_NULL_HANDLE)
    {
        VkShaderModuleCreateInfo frag_info = {};
        frag_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        frag_info.codeSize = sizeof(__glsl_shader_frag_spv);
        frag_info.pCode = (uint32_t*)__glsl_shader_frag_spv;
        //frag_info.codeSize = sizeof(__sigil_vk_tut_frag);
        //frag_info.pCode = (uint32_t*)__sigil_vk_tut_frag;
        VkResult err = vkCreateShaderModule(device, &frag_info, allocator, &bd->ShaderModuleFrag);
        sigil::visor::check_vk_result(err);
    }
    // if (bd->some_shader == VK_NULL_HANDLE)
    // {
    //     VkShaderModuleCreateInfo frag_info = {};
    //     frag_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    //     //frag_info.codeSize = sizeof(__glsl_shader_frag_spv);
    //     //frag_info.pCode = (uint32_t*)__glsl_shader_frag_spv;
    //     frag_info.codeSize = sizeof(__sigil_vk_tut_frag);
    //     frag_info.pCode = (uint32_t*)__sigil_vk_tut_frag;
    //     VkResult err = vkCreateShaderModule(device, &frag_info, allocator, &bd->some_shader);
    //     sigil::visor::check_vk_result(err);
    // }
}

static void ImGui_ImplVulkan_CreatePipeline(VkDevice device, const VkAllocationCallbacks* allocator, VkPipelineCache pipelineCache, VkRenderPass renderPass, VkSampleCountFlagBits MSAASamples, VkPipeline* pipeline, uint32_t subpass)
{
    printf("visor: creating pipeline for Vulkan\n");
    ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
    ImGui_ImplVulkan_CreateShaderModules(device, allocator);

    printf("visor: setting stage shaders\n");
    VkPipelineShaderStageCreateInfo stage[2] = {};
    stage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage[0].module = bd->ShaderModuleVert;
    stage[0].pName = "main";

    stage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stage[1].module = bd->ShaderModuleFrag;
    stage[1].pName = "main";

    // stage[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    // stage[2].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    // stage[2].module = bd->some_shader;
    // stage[2].pName = "main";

    //printf("visor: yyyy1\n");
    VkVertexInputBindingDescription binding_desc[1] = {};
    binding_desc[0].stride = sizeof(ImDrawVert);
    binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    //printf("visor: yyyy2\n");
    VkVertexInputAttributeDescription attribute_desc[3] = {};
    attribute_desc[0].location = 0;
    attribute_desc[0].binding = binding_desc[0].binding;
    attribute_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_desc[0].offset = offsetof(ImDrawVert, pos);
    attribute_desc[1].location = 1;
    attribute_desc[1].binding = binding_desc[0].binding;
    attribute_desc[1].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_desc[1].offset = offsetof(ImDrawVert, uv);
    attribute_desc[2].location = 2;
    attribute_desc[2].binding = binding_desc[0].binding;
    attribute_desc[2].format = VK_FORMAT_R8G8B8A8_UNORM;
    attribute_desc[2].offset = offsetof(ImDrawVert, col);

    //printf("visor: yyyy3\n");
    VkPipelineVertexInputStateCreateInfo vertex_info = {};
    vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_info.vertexBindingDescriptionCount = 1;
    vertex_info.pVertexBindingDescriptions = binding_desc;
    vertex_info.vertexAttributeDescriptionCount = 3;
    vertex_info.pVertexAttributeDescriptions = attribute_desc;

    //printf("visor: yyyy4\n");
    VkPipelineInputAssemblyStateCreateInfo ia_info = {};
    ia_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    //printf("visor: yyyy5\n");
    VkPipelineViewportStateCreateInfo viewport_info = {};
    viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_info.viewportCount = 1;
    viewport_info.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo raster_info = {};
    raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster_info.polygonMode = VK_POLYGON_MODE_FILL;
    raster_info.cullMode = VK_CULL_MODE_NONE;
    raster_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    raster_info.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms_info = {};
    ms_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms_info.rasterizationSamples = (MSAASamples != 0) ? MSAASamples : VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_attachment[1] = {};
    color_attachment[0].blendEnable = VK_TRUE;
    color_attachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_attachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_attachment[0].colorBlendOp = VK_BLEND_OP_ADD;
    color_attachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_attachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_attachment[0].alphaBlendOp = VK_BLEND_OP_ADD;
    color_attachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineDepthStencilStateCreateInfo depth_info = {};
    depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    VkPipelineColorBlendStateCreateInfo blend_info = {};
    blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blend_info.attachmentCount = 1;
    blend_info.pAttachments = color_attachment;

    VkDynamicState dynamic_states[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = (uint32_t)IM_ARRAYSIZE(dynamic_states);
    dynamic_state.pDynamicStates = dynamic_states;

    printf("visor: creating Vulkan pipeline create info\n");
    VkGraphicsPipelineCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    info.flags = bd->PipelineCreateFlags;
    info.stageCount = 2;
    info.pStages = stage;
    info.pVertexInputState = &vertex_info;
    info.pInputAssemblyState = &ia_info;
    info.pViewportState = &viewport_info;
    info.pRasterizationState = &raster_info;
    info.pMultisampleState = &ms_info;
    info.pDepthStencilState = &depth_info;
    info.pColorBlendState = &blend_info;
    info.pDynamicState = &dynamic_state;
    info.layout = bd->PipelineLayout;
    info.renderPass = renderPass;
    info.subpass = subpass;

#ifdef IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
    if (bd->VulkanInitInfo.UseDynamicRendering)
    {
        IM_ASSERT(bd->VulkanInitInfo.PipelineRenderingCreateInfo.sType == VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR && "PipelineRenderingCreateInfo sType must be VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR");
        IM_ASSERT(bd->VulkanInitInfo.PipelineRenderingCreateInfo.pNext == nullptr && "PipelineRenderingCreateInfo pNext must be NULL");
        info.pNext = &bd->VulkanInitInfo.PipelineRenderingCreateInfo;
        info.renderPass = VK_NULL_HANDLE; // Just make sure it's actually nullptr.
    }
#endif
    printf("visor: calling Vulkan API to create pipe\n");
    assert(device != nullptr);
    assert(pipelineCache == VK_NULL_HANDLE);
    VkResult err = vkCreateGraphicsPipelines(device, pipelineCache, 1, &info, allocator, pipeline);
    sigil::visor::check_vk_result(err);
}

bool ImGui_ImplVulkan_CreateDeviceObjects() {
    ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
    sigil::visor::vk_init_info_t* v = &bd->VulkanInitInfo;
    VkResult err;

    assert(bd != NULL);
    assert(v != NULL);

    if (!bd->FontSampler)
    {
        printf("visor: CreateDeviceObjects bilinear sampling\n");
        // Bilinear sampling is required by default. Set 'io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to allow point/nearest sampling.
        VkSamplerCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_LINEAR;
        info.minFilter = VK_FILTER_LINEAR;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.minLod = -1000;
        info.maxLod = 1000;
        info.maxAnisotropy = 1.0f;
        err = vkCreateSampler(v->Device, &info, v->Allocator, &bd->FontSampler);
        sigil::visor::check_vk_result(err);
    }

    if (!bd->DescriptorSetLayout)
    {
        printf("visor: CreateDeviceObjects descrtiptor set layour binding\n");
        VkDescriptorSetLayoutBinding binding[1] = {};
        binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding[0].descriptorCount = 1;
        binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        VkDescriptorSetLayoutCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = 1;
        info.pBindings = binding;
        err = vkCreateDescriptorSetLayout(v->Device, &info, v->Allocator, &bd->DescriptorSetLayout);
        sigil::visor::check_vk_result(err);
    }

    if (!bd->PipelineLayout)
    {
        // Constants: we are using 'vec2 offset' and 'vec2 scale' instead of a full 3d projection matrix
        printf("visor: CreateDeviceObjects pipeline layout\n");
        VkPushConstantRange push_constants[1] = {};
        push_constants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        push_constants[0].offset = sizeof(float) * 0;
        push_constants[0].size = sizeof(float) * 4;
        VkDescriptorSetLayout set_layout[1] = { bd->DescriptorSetLayout };
        VkPipelineLayoutCreateInfo layout_info = {};
        layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_info.setLayoutCount = 1;
        layout_info.pSetLayouts = set_layout;
        layout_info.pushConstantRangeCount = 1;
        layout_info.pPushConstantRanges = push_constants;
        err = vkCreatePipelineLayout(v->Device, &layout_info, v->Allocator, &bd->PipelineLayout);
        sigil::visor::check_vk_result(err);
    }

    ImGui_ImplVulkan_CreatePipeline(v->Device, v->Allocator, v->PipelineCache, v->RenderPass, v->MSAASamples, &bd->Pipeline, v->Subpass);

    return true;
}

void    ImGui_ImplVulkan_DestroyDeviceObjects()
{
    ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
    sigil::visor::vk_init_info_t* v = &bd->VulkanInitInfo;
    ImGui_ImplVulkan_DestroyWindowRenderBuffers(v->Device, &bd->MainWindowRenderBuffers, v->Allocator);
    ImGui_ImplVulkan_DestroyFontsTexture();

    if (bd->FontCommandBuffer)    { vkFreeCommandBuffers(v->Device, bd->FontCommandPool, 1, &bd->FontCommandBuffer); bd->FontCommandBuffer = VK_NULL_HANDLE; }
    if (bd->FontCommandPool)      { vkDestroyCommandPool(v->Device, bd->FontCommandPool, v->Allocator); bd->FontCommandPool = VK_NULL_HANDLE; }
    if (bd->ShaderModuleVert)     { vkDestroyShaderModule(v->Device, bd->ShaderModuleVert, v->Allocator); bd->ShaderModuleVert = VK_NULL_HANDLE; }
    if (bd->ShaderModuleFrag)     { vkDestroyShaderModule(v->Device, bd->ShaderModuleFrag, v->Allocator); bd->ShaderModuleFrag = VK_NULL_HANDLE; }
    if (bd->FontSampler)          { vkDestroySampler(v->Device, bd->FontSampler, v->Allocator); bd->FontSampler = VK_NULL_HANDLE; }
    if (bd->DescriptorSetLayout)  { vkDestroyDescriptorSetLayout(v->Device, bd->DescriptorSetLayout, v->Allocator); bd->DescriptorSetLayout = VK_NULL_HANDLE; }
    if (bd->PipelineLayout)       { vkDestroyPipelineLayout(v->Device, bd->PipelineLayout, v->Allocator); bd->PipelineLayout = VK_NULL_HANDLE; }
    if (bd->Pipeline)             { vkDestroyPipeline(v->Device, bd->Pipeline, v->Allocator); bd->Pipeline = VK_NULL_HANDLE; }
}

bool    ImGui_ImplVulkan_LoadFunctions(PFN_vkVoidFunction(*loader_func)(const char* function_name, void* user_data), void* user_data)
{
    // Load function pointers
    // You can use the default Vulkan loader using:
    //      ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void*) { return vkGetInstanceProcAddr(your_vk_isntance, function_name); });
    // But this would be equivalent to not setting VK_NO_PROTOTYPES.
#ifdef VK_NO_PROTOTYPES
#define IMGUI_VULKAN_FUNC_LOAD(func) \
    func = reinterpret_cast<decltype(func)>(loader_func(#func, user_data)); \
    if (func == nullptr)   \
        return false;
    IMGUI_VULKAN_FUNC_MAP(IMGUI_VULKAN_FUNC_LOAD)
#undef IMGUI_VULKAN_FUNC_LOAD

#ifdef IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
    // Manually load those two (see #5446)
    ImGuiImplVulkanFuncs_vkCmdBeginRenderingKHR = reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(loader_func("vkCmdBeginRenderingKHR", user_data));
    ImGuiImplVulkanFuncs_vkCmdEndRenderingKHR = reinterpret_cast<PFN_vkCmdEndRenderingKHR>(loader_func("vkCmdEndRenderingKHR", user_data));
#endif
#else
    IM_UNUSED(loader_func);
    IM_UNUSED(user_data);
#endif

    g_FunctionsLoaded = true;
    return true;
}

bool sigil::visor::vk_init(sigil::visor::vk_init_info_t* info) {
    IM_ASSERT(g_FunctionsLoaded && "Need to call ImGui_ImplVulkan_LoadFunctions() if IMGUI_IMPL_VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!");

    if (info->UseDynamicRendering)
    {
#ifdef IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
#ifndef VK_NO_PROTOTYPES
        ImGuiImplVulkanFuncs_vkCmdBeginRenderingKHR = reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(vkGetInstanceProcAddr(info->Instance, "vkCmdBeginRenderingKHR"));
        ImGuiImplVulkanFuncs_vkCmdEndRenderingKHR = reinterpret_cast<PFN_vkCmdEndRenderingKHR>(vkGetInstanceProcAddr(info->Instance, "vkCmdEndRenderingKHR"));
#endif
        IM_ASSERT(ImGuiImplVulkanFuncs_vkCmdBeginRenderingKHR != nullptr);
        IM_ASSERT(ImGuiImplVulkanFuncs_vkCmdEndRenderingKHR != nullptr);
#else
        IM_ASSERT(0 && "Can't use dynamic rendering when neither VK_VERSION_1_3 or VK_KHR_dynamic_rendering is defined.");
#endif
    }

    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    // Setup backend capabilities flags
    ImGui_ImplVulkan_Data* bd = IM_NEW(ImGui_ImplVulkan_Data)();
    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName = "imgui_impl_vulkan";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.

    IM_ASSERT(info->Instance != VK_NULL_HANDLE);
    IM_ASSERT(info->PhysicalDevice != VK_NULL_HANDLE);
    IM_ASSERT(info->Device != VK_NULL_HANDLE);
    IM_ASSERT(info->Queue != VK_NULL_HANDLE);
    IM_ASSERT(info->DescriptorPool != VK_NULL_HANDLE);
    IM_ASSERT(info->MinImageCount >= 2);
    IM_ASSERT(info->ImageCount >= info->MinImageCount);
    if (info->UseDynamicRendering == false)
        IM_ASSERT(info->RenderPass != VK_NULL_HANDLE);

    bd->VulkanInitInfo = *info;

    printf("visor: creating device objects\n");
    ImGui_ImplVulkan_CreateDeviceObjects();

    return true;
}

void sigil::visor::ImGui_ImplVulkan_Shutdown()
{
    ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
    IM_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplVulkan_DestroyDeviceObjects();
    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    io.BackendFlags &= ~ImGuiBackendFlags_RendererHasVtxOffset;
    IM_DELETE(bd);
}

void ImGui_ImplVulkan_NewFrame()
{
    ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
    IM_ASSERT(bd != nullptr && "Did you call sigil::visor::vk_init()?");

    if (!bd->FontDescriptorSet)
        ImGui_ImplVulkan_CreateFontsTexture();
}

void ImGui_ImplVulkan_SetMinImageCount(uint32_t min_image_count)
{
    ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
    IM_ASSERT(min_image_count >= 2);
    if (bd->VulkanInitInfo.MinImageCount == min_image_count)
        return;

    sigil::visor::vk_init_info_t* v = &bd->VulkanInitInfo;
    VkResult err = vkDeviceWaitIdle(v->Device);
    sigil::visor::check_vk_result(err);
    ImGui_ImplVulkan_DestroyWindowRenderBuffers(v->Device, &bd->MainWindowRenderBuffers, v->Allocator);
    bd->VulkanInitInfo.MinImageCount = min_image_count;
}

// Register a texture
// FIXME: This is experimental in the sense that we are unsure how to best design/tackle this problem, please post to https://github.com/ocornut/imgui/pull/914 if you have suggestions.
VkDescriptorSet ImGui_ImplVulkan_AddTexture(VkSampler sampler, VkImageView image_view, VkImageLayout image_layout)
{
    ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
    sigil::visor::vk_init_info_t* v = &bd->VulkanInitInfo;

    // Create Descriptor Set:
    VkDescriptorSet descriptor_set;
    {
        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = v->DescriptorPool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &bd->DescriptorSetLayout;
        VkResult err = vkAllocateDescriptorSets(v->Device, &alloc_info, &descriptor_set);
        sigil::visor::check_vk_result(err);
    }

    // Update the Descriptor Set:
    {
        VkDescriptorImageInfo desc_image[1] = {};
        desc_image[0].sampler = sampler;
        desc_image[0].imageView = image_view;
        desc_image[0].imageLayout = image_layout;
        VkWriteDescriptorSet write_desc[1] = {};
        write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc[0].dstSet = descriptor_set;
        write_desc[0].descriptorCount = 1;
        write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_desc[0].pImageInfo = desc_image;
        vkUpdateDescriptorSets(v->Device, 1, write_desc, 0, nullptr);
    }
    return descriptor_set;
}

void ImGui_ImplVulkan_RemoveTexture(VkDescriptorSet descriptor_set)
{
    ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
    sigil::visor::vk_init_info_t* v = &bd->VulkanInitInfo;
    vkFreeDescriptorSets(v->Device, v->DescriptorPool, 1, &descriptor_set);
}

void ImGui_ImplVulkan_DestroyFrameRenderBuffers(VkDevice device, ImGui_ImplVulkan_FrameRenderBuffers* buffers, const VkAllocationCallbacks* allocator)
{
    if (buffers->VertexBuffer) { vkDestroyBuffer(device, buffers->VertexBuffer, allocator); buffers->VertexBuffer = VK_NULL_HANDLE; }
    if (buffers->VertexBufferMemory) { vkFreeMemory(device, buffers->VertexBufferMemory, allocator); buffers->VertexBufferMemory = VK_NULL_HANDLE; }
    if (buffers->IndexBuffer) { vkDestroyBuffer(device, buffers->IndexBuffer, allocator); buffers->IndexBuffer = VK_NULL_HANDLE; }
    if (buffers->IndexBufferMemory) { vkFreeMemory(device, buffers->IndexBufferMemory, allocator); buffers->IndexBufferMemory = VK_NULL_HANDLE; }
    buffers->VertexBufferSize = 0;
    buffers->IndexBufferSize = 0;
}

void ImGui_ImplVulkan_DestroyWindowRenderBuffers(VkDevice device, ImGui_ImplVulkan_WindowRenderBuffers* buffers, const VkAllocationCallbacks* allocator)
{
    for (uint32_t n = 0; n < buffers->Count; n++)
        ImGui_ImplVulkan_DestroyFrameRenderBuffers(device, &buffers->FrameRenderBuffers[n], allocator);
    IM_FREE(buffers->FrameRenderBuffers);
    buffers->FrameRenderBuffers = nullptr;
    buffers->Index = 0;
    buffers->Count = 0;
}

//-------------------------------------------------------------------------
// Internal / Miscellaneous Vulkan Helpers
// (Used by example's main.cpp. Used by multi-viewport features. PROBABLY NOT used by your own app.)
//-------------------------------------------------------------------------
// You probably do NOT need to use or care about those functions.
// Those functions only exist because:
//   1) they facilitate the readability and maintenance of the multiple main.cpp examples files.
//   2) the upcoming multi-viewport feature will need them internally.
// Generally we avoid exposing any kind of superfluous high-level helpers in the backends,
// but it is too much code to duplicate everywhere so we exceptionally expose them.
//
// Your engine/app will likely _already_ have code to setup all that stuff (swap chain, render pass, frame buffers, etc.).
// You may read this code to learn about Vulkan, but it is recommended you use you own custom tailored code to do equivalent work.
// (The ImGui_ImplVulkanH_XXX functions do not interact with any of the state used by the regular ImGui_ImplVulkan_XXX functions)
//-------------------------------------------------------------------------

VkSurfaceFormatKHR ImGui_ImplVulkanH_SelectSurfaceFormat(VkPhysicalDevice physical_device, VkSurfaceKHR surface, const VkFormat* request_formats, int request_formats_count, VkColorSpaceKHR request_color_space)
{
    IM_ASSERT(g_FunctionsLoaded && "Need to call ImGui_ImplVulkan_LoadFunctions() if IMGUI_IMPL_VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!");
    IM_ASSERT(request_formats != nullptr);
    IM_ASSERT(request_formats_count > 0);

    // Per Spec Format and View Format are expected to be the same unless VK_IMAGE_CREATE_MUTABLE_BIT was set at image creation
    // Assuming that the default behavior is without setting this bit, there is no need for separate Swapchain image and image view format
    // Additionally several new color spaces were introduced with Vulkan Spec v1.0.40,
    // hence we must make sure that a format with the mostly available color space, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, is found and used.
    uint32_t avail_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &avail_count, nullptr);
    ImVector<VkSurfaceFormatKHR> avail_format;
    avail_format.resize((int)avail_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &avail_count, avail_format.Data);

    // First check if only one format, VK_FORMAT_UNDEFINED, is available, which would imply that any format is available
    if (avail_count == 1)
    {
        if (avail_format[0].format == VK_FORMAT_UNDEFINED)
        {
            VkSurfaceFormatKHR ret;
            ret.format = request_formats[0];
            ret.colorSpace = request_color_space;
            return ret;
        }
        else
        {
            // No point in searching another format
            return avail_format[0];
        }
    }
    else
    {
        // Request several formats, the first found will be used
        for (int request_i = 0; request_i < request_formats_count; request_i++)
            for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
                if (avail_format[avail_i].format == request_formats[request_i] && avail_format[avail_i].colorSpace == request_color_space)
                    return avail_format[avail_i];

        // If none of the requested image formats could be found, use the first available
        return avail_format[0];
    }
}

VkPresentModeKHR ImGui_ImplVulkanH_SelectPresentMode(VkPhysicalDevice physical_device, VkSurfaceKHR surface, const VkPresentModeKHR* request_modes, int request_modes_count)
{
    IM_ASSERT(g_FunctionsLoaded && "Need to call ImGui_ImplVulkan_LoadFunctions() if IMGUI_IMPL_VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!");
    IM_ASSERT(request_modes != nullptr);
    IM_ASSERT(request_modes_count > 0);

    // Request a certain mode and confirm that it is available. If not use VK_PRESENT_MODE_FIFO_KHR which is mandatory
    uint32_t avail_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &avail_count, nullptr);
    ImVector<VkPresentModeKHR> avail_modes;
    avail_modes.resize((int)avail_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &avail_count, avail_modes.Data);
    //for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
    //    printf("[vulkan] avail_modes[%d] = %d\n", avail_i, avail_modes[avail_i]);

    for (int request_i = 0; request_i < request_modes_count; request_i++)
        for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
            if (request_modes[request_i] == avail_modes[avail_i])
                return request_modes[request_i];

    return VK_PRESENT_MODE_FIFO_KHR; // Always available
}

void ImGui_ImplVulkanH_CreateWindowCommandBuffers(VkPhysicalDevice physical_device, VkDevice device, sigil::visor::vk_window_t* wd, uint32_t queue_family, const VkAllocationCallbacks* allocator)
{
    IM_ASSERT(physical_device != VK_NULL_HANDLE && device != VK_NULL_HANDLE);
    IM_UNUSED(physical_device);

    // Create Command Buffers
    VkResult err;
    for (uint32_t i = 0; i < wd->ImageCount; i++)
    {
        sigil::visor::vk_framedata_t* fd = &wd->Frames[i];
        {
            VkCommandPoolCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            info.flags = 0;
            info.queueFamilyIndex = queue_family;
            err = vkCreateCommandPool(device, &info, allocator, &fd->CommandPool);
            sigil::visor::check_vk_result(err);
        }
        {
            VkCommandBufferAllocateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.commandPool = fd->CommandPool;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            info.commandBufferCount = 1;
            err = vkAllocateCommandBuffers(device, &info, &fd->CommandBuffer);
            sigil::visor::check_vk_result(err);
        }
        {
            VkFenceCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            err = vkCreateFence(device, &info, allocator, &fd->Fence);
            sigil::visor::check_vk_result(err);
        }
    }

    for (uint32_t i = 0; i < wd->SemaphoreCount; i++)
    {
        sigil::visor::vk_frame_semaphores_t* fsd = &wd->FrameSemaphores[i];
        {
            VkSemaphoreCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            err = vkCreateSemaphore(device, &info, allocator, &fsd->ImageAcquiredSemaphore);
            sigil::visor::check_vk_result(err);
            err = vkCreateSemaphore(device, &info, allocator, &fsd->RenderCompleteSemaphore);
            sigil::visor::check_vk_result(err);
        }
    }
}

int ImGui_ImplVulkanH_GetMinImageCountFromPresentMode(VkPresentModeKHR present_mode)
{
    if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
        return 3;
    if (present_mode == VK_PRESENT_MODE_FIFO_KHR || present_mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
        return 2;
    if (present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        return 1;
    IM_ASSERT(0);
    return 1;
}

// Also destroy old swap chain and in-flight frames data, if any.
void ImGui_ImplVulkanH_CreateWindowSwapChain(VkPhysicalDevice physical_device, VkDevice device, sigil::visor::vk_window_t* wd, const VkAllocationCallbacks* allocator, int w, int h, uint32_t min_image_count)
{
    VkResult err;
    VkSwapchainKHR old_swapchain = wd->Swapchain;
    wd->Swapchain = VK_NULL_HANDLE;
    err = vkDeviceWaitIdle(device);
    sigil::visor::check_vk_result(err);

    // We don't use ImGui_ImplVulkanH_DestroyWindow() because we want to preserve the old swapchain to create the new one.
    // Destroy old Framebuffer
    for (uint32_t i = 0; i < wd->ImageCount; i++)
        ImGui_ImplVulkanH_DestroyFrame(device, &wd->Frames[i], allocator);
    for (uint32_t i = 0; i < wd->SemaphoreCount; i++)
        ImGui_ImplVulkanH_DestroyFrameSemaphores(device, &wd->FrameSemaphores[i], allocator);
    IM_FREE(wd->Frames);
    IM_FREE(wd->FrameSemaphores);
    wd->Frames = nullptr;
    wd->FrameSemaphores = nullptr;
    wd->ImageCount = 0;
    if (wd->RenderPass)
        vkDestroyRenderPass(device, wd->RenderPass, allocator);
    if (wd->Pipeline)
        vkDestroyPipeline(device, wd->Pipeline, allocator);

    // If min image count was not specified, request different count of images dependent on selected present mode
    if (min_image_count == 0)
        min_image_count = ImGui_ImplVulkanH_GetMinImageCountFromPresentMode(wd->PresentMode);

    // Create Swapchain
    {
        VkSwapchainCreateInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = wd->Surface;
        info.minImageCount = min_image_count;
        info.imageFormat = wd->SurfaceFormat.format;
        info.imageColorSpace = wd->SurfaceFormat.colorSpace;
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;           // Assume that graphics family == present family
        info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = wd->PresentMode;
        info.clipped = VK_TRUE;
        info.oldSwapchain = old_swapchain;
        VkSurfaceCapabilitiesKHR cap;
        err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, wd->Surface, &cap);
        sigil::visor::check_vk_result(err);
        if (info.minImageCount < cap.minImageCount)
            info.minImageCount = cap.minImageCount;
        else if (cap.maxImageCount != 0 && info.minImageCount > cap.maxImageCount)
            info.minImageCount = cap.maxImageCount;

        if (cap.currentExtent.width == 0xffffffff)
        {
            info.imageExtent.width = wd->Width = w;
            info.imageExtent.height = wd->Height = h;
        }
        else
        {
            info.imageExtent.width = wd->Width = cap.currentExtent.width;
            info.imageExtent.height = wd->Height = cap.currentExtent.height;
        }
        err = vkCreateSwapchainKHR(device, &info, allocator, &wd->Swapchain);
        sigil::visor::check_vk_result(err);
        err = vkGetSwapchainImagesKHR(device, wd->Swapchain, &wd->ImageCount, nullptr);
        sigil::visor::check_vk_result(err);
        VkImage backbuffers[16] = {};
        IM_ASSERT(wd->ImageCount >= min_image_count);
        IM_ASSERT(wd->ImageCount < IM_ARRAYSIZE(backbuffers));
        err = vkGetSwapchainImagesKHR(device, wd->Swapchain, &wd->ImageCount, backbuffers);
        sigil::visor::check_vk_result(err);

        IM_ASSERT(wd->Frames == nullptr && wd->FrameSemaphores == nullptr);
        wd->SemaphoreCount = wd->ImageCount + 1;
        wd->Frames = (sigil::visor::vk_framedata_t*)IM_ALLOC(sizeof(sigil::visor::vk_framedata_t) * wd->ImageCount);
        wd->FrameSemaphores = (sigil::visor::vk_frame_semaphores_t*)IM_ALLOC(sizeof(sigil::visor::vk_frame_semaphores_t) * wd->SemaphoreCount);
        memset(wd->Frames, 0, sizeof(wd->Frames[0]) * wd->ImageCount);
        memset(wd->FrameSemaphores, 0, sizeof(wd->FrameSemaphores[0]) * wd->SemaphoreCount);
        for (uint32_t i = 0; i < wd->ImageCount; i++)
            wd->Frames[i].Backbuffer = backbuffers[i];
    }
    if (old_swapchain)
        vkDestroySwapchainKHR(device, old_swapchain, allocator);

    // Create the Render Pass
    if (wd->UseDynamicRendering == false)
    {
        VkAttachmentDescription attachment = {};
        attachment.format = wd->SurfaceFormat.format;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = wd->ClearEnable ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference color_attachment = {};
        color_attachment.attachment = 0;
        color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment;
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        VkRenderPassCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount = 1;
        info.pAttachments = &attachment;
        info.subpassCount = 1;
        info.pSubpasses = &subpass;
        info.dependencyCount = 1;
        info.pDependencies = &dependency;
        err = vkCreateRenderPass(device, &info, allocator, &wd->RenderPass);
        sigil::visor::check_vk_result(err);

        // We do not create a pipeline by default as this is also used by examples' main.cpp,
        // but secondary viewport in multi-viewport mode may want to create one with:
        //ImGui_ImplVulkan_CreatePipeline(device, allocator, VK_NULL_HANDLE, wd->RenderPass, VK_SAMPLE_COUNT_1_BIT, &wd->Pipeline, v->Subpass);
    }

    // Create The Image Views
    {
        VkImageViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = wd->SurfaceFormat.format;
        info.components.r = VK_COMPONENT_SWIZZLE_R;
        info.components.g = VK_COMPONENT_SWIZZLE_G;
        info.components.b = VK_COMPONENT_SWIZZLE_B;
        info.components.a = VK_COMPONENT_SWIZZLE_A;
        VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        info.subresourceRange = image_range;
        for (uint32_t i = 0; i < wd->ImageCount; i++)
        {
            sigil::visor::vk_framedata_t* fd = &wd->Frames[i];
            info.image = fd->Backbuffer;
            err = vkCreateImageView(device, &info, allocator, &fd->BackbufferView);
            sigil::visor::check_vk_result(err);
        }
    }

    // Create Framebuffer
    if (wd->UseDynamicRendering == false)
    {
        VkImageView attachment[1];
        VkFramebufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = wd->RenderPass;
        info.attachmentCount = 1;
        info.pAttachments = attachment;
        info.width = wd->Width;
        info.height = wd->Height;
        info.layers = 1;
        for (uint32_t i = 0; i < wd->ImageCount; i++)
        {
            sigil::visor::vk_framedata_t* fd = &wd->Frames[i];
            attachment[0] = fd->BackbufferView;
            err = vkCreateFramebuffer(device, &info, allocator, &fd->Framebuffer);
            sigil::visor::check_vk_result(err);
        }
    }
}

// Create or resize window
void ImGui_ImplVulkanH_CreateOrResizeWindow(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device, sigil::visor::vk_window_t* wd, uint32_t queue_family, const VkAllocationCallbacks* allocator, int width, int height, uint32_t min_image_count)
{
    IM_ASSERT(g_FunctionsLoaded && "Need to call ImGui_ImplVulkan_LoadFunctions() if IMGUI_IMPL_VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!");
    (void)instance;
    ImGui_ImplVulkanH_CreateWindowSwapChain(physical_device, device, wd, allocator, width, height, min_image_count);
    ImGui_ImplVulkanH_CreateWindowCommandBuffers(physical_device, device, wd, queue_family, allocator);
}

void ImGui_ImplVulkanH_DestroyWindow(VkInstance instance, VkDevice device, sigil::visor::vk_window_t* wd, const VkAllocationCallbacks* allocator)
{
    vkDeviceWaitIdle(device); // FIXME: We could wait on the Queue if we had the queue in wd-> (otherwise VulkanH functions can't use globals)
    //vkQueueWaitIdle(bd->Queue);

    for (uint32_t i = 0; i < wd->ImageCount; i++)
        ImGui_ImplVulkanH_DestroyFrame(device, &wd->Frames[i], allocator);
    for (uint32_t i = 0; i < wd->SemaphoreCount; i++)
        ImGui_ImplVulkanH_DestroyFrameSemaphores(device, &wd->FrameSemaphores[i], allocator);
    IM_FREE(wd->Frames);
    IM_FREE(wd->FrameSemaphores);
    wd->Frames = nullptr;
    wd->FrameSemaphores = nullptr;
    vkDestroyPipeline(device, wd->Pipeline, allocator);
    vkDestroyRenderPass(device, wd->RenderPass, allocator);
    vkDestroySwapchainKHR(device, wd->Swapchain, allocator);
    vkDestroySurfaceKHR(instance, wd->Surface, allocator);

    *wd = sigil::visor::vk_window_t();
}

void ImGui_ImplVulkanH_DestroyFrame(VkDevice device, sigil::visor::vk_framedata_t* fd, const VkAllocationCallbacks* allocator)
{
    vkDestroyFence(device, fd->Fence, allocator);
    vkFreeCommandBuffers(device, fd->CommandPool, 1, &fd->CommandBuffer);
    vkDestroyCommandPool(device, fd->CommandPool, allocator);
    fd->Fence = VK_NULL_HANDLE;
    fd->CommandBuffer = VK_NULL_HANDLE;
    fd->CommandPool = VK_NULL_HANDLE;

    vkDestroyImageView(device, fd->BackbufferView, allocator);
    vkDestroyFramebuffer(device, fd->Framebuffer, allocator);
}

void ImGui_ImplVulkanH_DestroyFrameSemaphores(VkDevice device, sigil::visor::vk_frame_semaphores_t* fsd, const VkAllocationCallbacks* allocator)
{
    vkDestroySemaphore(device, fsd->ImageAcquiredSemaphore, allocator);
    vkDestroySemaphore(device, fsd->RenderCompleteSemaphore, allocator);
    fsd->ImageAcquiredSemaphore = fsd->RenderCompleteSemaphore = VK_NULL_HANDLE;
}


bool sigil::visor::is_vk_ext_available(const std::vector<VkExtensionProperties> &properties, const char* extension) {
    for (const VkExtensionProperties& p : properties) {
        if (strcmp(p.extensionName, extension) == 0) return true;
    }

    return false;
}

void sigil::visor::vk_auto_select_gpu() {
    if (!sigil::visor::data) return;

    printf("visor: selecting GPU\n");
    VkResult err = vkEnumeratePhysicalDevices(sigil::visor::data->vk_inst, 
                    &sigil::visor::data->num_phys, nullptr);
    sigil::visor::check_vk_result(err);
    //IM_ASSERT(gpu_count > 0);

    sigil::visor::data->vk_phy_all.resize(sigil::visor::data->num_phys);
    err = vkEnumeratePhysicalDevices(sigil::visor::data->vk_inst, 
                                 &sigil::visor::data->num_phys, 
                                     sigil::visor::data->vk_phy_all.data());
    sigil::visor::check_vk_result(err);

    for (VkPhysicalDevice& device : sigil::visor::data->vk_phy_all) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            sigil::visor::data->vk_phy_active = device;
            printf("visor: Discrete GPU found\n");
            return;
        }
    }

    // Use first GPU (Integrated) is a Discrete one is not available.
    if (sigil::visor::data->num_phys > 0) sigil::visor::data->vk_phy_active = sigil::visor::data->vk_phy_all.at(0);
}

/*
    Starting Vulkan:
        Instance and physical device
        Logical device and queues
        Window  Surface (GLFW)
        Image Views and Frame Buffers
        Render Pass
        Graphics Pipeline
        Command Poools, Command buffers
*/

int sigil::visor::vk_inst_create(std::vector<const char*> vk_inst_ext) {
    assert(sigil::visor::data != NULL);
    std::vector<VkExtensionProperties> properties;
    uint32_t properties_count;
    VkResult err;

    printf("visor: trying to create Vulkan instance...\n");
    // Create Vulkan Instance
    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
    properties.resize(properties_count);
    err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.data());
    sigil::visor::check_vk_result(err);

    printf("visor: selecting Vulkan extensions...\n");
    if (is_vk_ext_available(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        vk_inst_ext.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

#   ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    if (is_vk_ext_available(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        vk_inst_ext.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#   endif

#   ifdef APP_USE_VULKAN_DEBUG_REPORT
    const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
    create_info.enabledLayerCount = 1;
    create_info.ppEnabledLayexprerNames = layers;
    vk_inst_ext.push_back("VK_EXT_debug_report");
#   endif

    create_info.enabledExtensionCount = (uint32_t)vk_inst_ext.size();
    create_info.ppEnabledExtensionNames = vk_inst_ext.data();
    printf("visor: enabled Vulkan extensions: %u\n", create_info.enabledExtensionCount);
    assert(sigil::visor::data != NULL);
    assert(sigil::visor::data->vk_allocators == NULL);
    assert(sigil::visor::data->vk_inst == NULL);
    err = vkCreateInstance(&create_info, sigil::visor::data->vk_allocators, &sigil::visor::data->vk_inst);
    sigil::visor::check_vk_result(err);
    printf("visor: Vulkan instance created successfully\n");

#   ifdef APP_USE_VULKAN_DEBUG_REPORT
    auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkCreateDebugReportCallbackEXT");
    IM_ASSERT(vkCreateDebugReportCallbackEXT != nullptr);
    VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
    debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    debug_report_ci.pfnCallback = debug_report;
    debug_report_ci.pUserData = nullptr;
    err = vkCreateDebugReportCallbackEXT(g_Instance, &debug_report_ci, g_Allocator, &g_DebugReport);
    sigil::visor::check_vk_result(err);
#   endif
    return err;
}

int sigil::visor::vk_setup_queues() {
    std::vector<VkExtensionProperties> properties;
    uint32_t properties_count;
    uint32_t count;
    VkResult err;

    printf("visor: setting up Vulkan queues\n");
    assert(sigil::visor::data != nullptr);
    assert(sigil::visor::data->vk_phy_active != VK_NULL_HANDLE);

    vkGetPhysicalDeviceQueueFamilyProperties(sigil::visor::data->vk_phy_active, &count, nullptr);
    VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
    vkGetPhysicalDeviceQueueFamilyProperties(sigil::visor::data->vk_phy_active, &count, queues);

    for (uint32_t i = 0; i < count; i++) {
        if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            sigil::visor::data->vk_main_q.family = i;
            break;
        }
    }

    free(queues);
    assert(sigil::visor::data->vk_main_q.family != (uint32_t)-1);

    std::vector<const char*> vk_dev_ext;
    // Create Logical Device (with 1 queue)
    vk_dev_ext.push_back("VK_KHR_swapchain");
    properties_count = 0;
    properties = {};

    // Enumerate physical device extension
    vkEnumerateDeviceExtensionProperties(sigil::visor::data->vk_phy_active, nullptr, &properties_count, nullptr);
    properties.resize(properties_count);
    vkEnumerateDeviceExtensionProperties(sigil::visor::data->vk_phy_active, nullptr, &properties_count, properties.data());
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    if (is_vk_ext_available(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
        vk_dev_ext.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

    const float queue_priority[] = { 1.0f };
    VkDeviceQueueCreateInfo queue_info[1] = {};
    queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex = sigil::visor::data->vk_main_q.family;
    queue_info[0].queueCount = 1;
    queue_info[0].pQueuePriorities = queue_priority;
    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
    create_info.pQueueCreateInfos = queue_info;
    create_info.enabledExtensionCount = (uint32_t)vk_dev_ext.size();
    create_info.ppEnabledExtensionNames = vk_dev_ext.data();
    err = vkCreateDevice(sigil::visor::data->vk_phy_active, &create_info, sigil::visor::data->vk_allocators, &sigil::visor::data->vk_dev);
    sigil::visor::check_vk_result(err);
    vkGetDeviceQueue(sigil::visor::data->vk_dev, sigil::visor::data->vk_main_q.family, 0, &sigil::visor::data->vk_main_q.queue);
    return err;
}

int sigil::visor::vk_setup_descriptor_pool() {
    VkResult err;

    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
    };
    
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = (uint32_t)sizeof(pool_info);
    pool_info.pPoolSizes = pool_sizes;
    err = vkCreateDescriptorPool(sigil::visor::data->vk_dev, &pool_info, sigil::visor::data->vk_allocators, &sigil::visor::data->vk_descriptor_pool);
    sigil::visor::check_vk_result(err);
    return err;;
}

int sigil::visor::set_vk_present_mode(bool vsync) {
    assert(sigil::visor::data->vk_phy_active != NULL);
    assert(sigil::visor::data->vk_window.Surface != NULL);

    std::vector<VkPresentModeKHR> present_modes = {};

    if (vsync) present_modes.push_back(VK_PRESENT_MODE_FIFO_KHR);
    else {
        present_modes.push_back(VK_PRESENT_MODE_MAILBOX_KHR);
        present_modes.push_back(VK_PRESENT_MODE_IMMEDIATE_KHR);
        present_modes.push_back(VK_PRESENT_MODE_FIFO_KHR);
    }

    sigil::visor::data->vk_window.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(sigil::visor::data->vk_phy_active, 
                            sigil::visor::data->vk_window.Surface, 
                            &present_modes[0], 
                            present_modes.size());
    printf("main: selected Vulkan PresentMode = %d\n",sigil::visor::data->vk_window.PresentMode);
    return 0;
}

int sigil::visor::init(void *ctx) {
    printf("visor: initializing\n");
    sigil::visor::data = new sigil::visor::module_data_t;
    sigil::visor::data->num_phys = 0;
    sigil::visor::data->vk_descriptor_pool = VK_NULL_HANDLE;
    sigil::visor::data->vk_pipeline_cache = VK_NULL_HANDLE;
    sigil::visor::data->vk_dev = VK_NULL_HANDLE;
    sigil::visor::data->vk_phy_active = VK_NULL_HANDLE;
    sigil::visor::data->vk_phy_aux = VK_NULL_HANDLE;
    sigil::visor::data->vk_phy_all = {};
    sigil::visor::data->vk_inst = VK_NULL_HANDLE;
    sigil::visor::data->vk_allocators = VK_NULL_HANDLE;
    sigil::visor::data->vk_main_q.family = 0;
    sigil::visor::data->vk_main_q.queue = VK_NULL_HANDLE;
    sigil::visor::data->vk_compute_q.family = 0;
    sigil::visor::data->vk_compute_q.queue = VK_NULL_HANDLE;
    sigil::visor::data->vk_window.Swapchain = VK_NULL_HANDLE;
    sigil::visor::data->vk_window.Surface = nullptr;
    sigil::visor::data->vk_window.glfw_window = nullptr;

     
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    VkResult vk_err;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    GLFWmonitor* primary = sigil::env::fullscreen ? glfwGetPrimaryMonitor() : NULL;
    sigil::visor::data->vk_window.glfw_window = glfwCreateWindow(mode->width, mode->height, sigil::env::hostname, primary, nullptr);

    if (!glfwVulkanSupported()) {
        printf("runtime: GLFW - Vulkan Not Supported\n");
        return 1;
    }

    std::vector<const char*> extensions;
    uint32_t extensions_count = 0;
    printf("visor: preparing %u Vulkan extensions for glfw\n", extensions_count);
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&extensions_count);

    for (uint32_t i = 0; i < extensions_count; i++) extensions.push_back(glfw_extensions[i]);


    sigil::visor::vk_inst_create(extensions);
    sigil::visor::vk_auto_select_gpu();
    sigil::visor::vk_setup_queues();
    sigil::visor::vk_setup_descriptor_pool();
    printf("visor: vulkan setup successfully\n");
    
    
    printf("main: creating glfw windows surface\n");
    vk_err = glfwCreateWindowSurface(sigil::visor::data->vk_inst, sigil::visor::data->vk_window.glfw_window, sigil::visor::data->vk_allocators, &sigil::visor::data->vk_window.Surface);
    sigil::visor::check_vk_result(vk_err);
    
    // Create Framebuffers
    int width, height;
    glfwGetFramebufferSize(sigil::visor::data->vk_window.glfw_window, &width, &height);
    printf("main: setting Vulkan window\n");

    VkBool32 res;
    //assert(sigil::visor::data->vk_queues.size() > 0);
    assert(sigil::visor::data->vk_phy_active != VK_NULL_HANDLE);
    vk_err = vkGetPhysicalDeviceSurfaceSupportKHR(sigil::visor::data->vk_phy_active,
                                                    sigil::visor::data->vk_main_q.family,
                                                    sigil::visor::data->vk_window.Surface, &res);
    sigil::visor::check_vk_result(vk_err);
    if (res != VK_TRUE) {
        fprintf(stderr, "main: Error -> WSI support on physical device 0\n");
        exit(-1);
    }


    // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    sigil::visor::data->vk_window.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(sigil::visor::data->vk_phy_active, sigil::visor::data->vk_window.Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

    set_vk_present_mode(sigil::visor::data->vsync);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    assert(sigil::visor::data->min_image_count >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(sigil::visor::data->vk_inst, 
                sigil::visor::data->vk_phy_active, sigil::visor::data->vk_dev, 
                &sigil::visor::data->vk_window, sigil::visor::data->vk_main_q.family, 
                sigil::visor::data->vk_allocators, width, height, sigil::visor::data->min_image_count);

    printf("main: vk_window setup succesfully\n");

    return 0;   
}

int sigil::visor::deinit(void *ctx) {
    VkResult vk_err = vkDeviceWaitIdle(sigil::visor::data->vk_dev);
    sigil::visor::check_vk_result(vk_err);
    sigil::visor::ImGui_ImplVulkan_Shutdown();

    ImGui::DestroyContext();

    ImGui_ImplVulkanH_DestroyWindow(sigil::visor::data->vk_inst, sigil::visor::data->vk_dev, 
                                    &sigil::visor::data->vk_window, sigil::visor::data->vk_allocators);

    vkDestroyDescriptorPool(sigil::visor::data->vk_dev, 
                    sigil::visor::data->vk_descriptor_pool, 
                    sigil::visor::data->vk_allocators);

#ifdef APP_USE_VULKAN_DEBUG_REPORT
    // Remove the debug report callback
    auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(sigil::visor::data->vk_inst, "vkDestroyDebugReportCallbackEXT");
    vkDestroyDebugReportCallbackEXT(sigil::visor::data->vk_inst, g_DebugReport, sigil::visor::data->vk_allocators);
#endif // APP_USE_VULKAN_DEBUG_REPORT

    vkDestroyDevice(sigil::visor::data->vk_dev, sigil::visor::data->vk_allocators);
    vkDestroyInstance(sigil::visor::data->vk_inst, sigil::visor::data->vk_allocators);
    glfwDestroyWindow(sigil::visor::data->vk_window.glfw_window);
    glfwTerminate();
    printf("visor: deinitialized\n");
    return 0;   
}

int sigil::visor::import() {
    sigil::modules::static_module_descriptor_t import_desc;

    printf("visor: attempting to import...\n");

    import_desc.name = "visor";
    import_desc.init = sigil::visor::init;
    import_desc.deinit = sigil::visor::deinit;

    sigil::visor::module = sigil::modules::import(import_desc, false);

    if (!sigil::visor::module) {
        printf("visor: import failed...\n");
        return -ENOMEM;
    }

    printf("visor: import success!\n");
    return 0;
}
