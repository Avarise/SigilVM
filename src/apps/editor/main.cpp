#include <cassert>
#include <cstddef>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdint>
#include <cstdlib>
#include <argp.h>
#include <vulkan/vulkan_core.h>

#include "../../core/runtime.h"
#include "../../core/memory.h"
#include "../../core/math.h"
#include "../../ntt/physics.h"
#include "../../ntt/hat.h"
#include "../../core/env.h"
#include "../../ntt/ntt.h"
#include "../../imgui/imgui.h"
#include "../../visor/visor.h"
#include "../../hid/hid.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

//#define APP_USE_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

const char *argp_program_version = "sigil-editor-1.0.0";
const char *argp_program_bug_address = "<sebastian.czapla.567@gmail.com>";
static char doc[] = "SigilVM scene editor";
static char args_doc[] = "[FILENAME]...";

sigil::memory::memstat_t memstats;

static struct argp_option options[] = { 
    { "debug", 'd', 0, 0, "Add debug printout"},
    { "fullscreen", 'k', 0, 0, "Start in fullscreen"},
    { 0 } 
};

struct private_state_t {
    ImVec4 clear_color = ImVec4(1.00f, 0.55f, 0.60f, 1.00f);
    enum theme {
        DARKMODE,
        LIGHTMODE,
        SYSTEM,
    } theme;
    int err;

    bool show_asset_manager_window = true;
    bool show_asset_editor_window = true;
    bool show_text_editor = true;
    bool show_performance_popup = true;
    bool show_overview_window = true;
    bool show_options_window = true;
    bool show_demo_window = false;
    bool show_spellslot_tracker = false;
    bool show_dice_roller = false;
    bool show_output_window = true;

    char text_editor_buffer[1024 * 512];
    char output_window_buffer[128][256] {
        "runtime: started in 0.92ms",
        "runtime: shaders compiled in 0.88ms",
        "roll: d20+4 && 2d6+5 resulted in 17 and 11"
    };

    struct dice_roller_data_t {
        int d4count = 0;
        int d6count = 0;
        int d8count = 0;
        int d10count = 0;
        int d12count = 0;
        int d20count = 0;
        int d4roll = 0;
        int d6roll = 0;
        int d8roll = 0;
        int d10roll = 0;
        int d12roll = 0;
        int d20roll = 0;
    } dice_roller_data;

    sigil::ntt::scene_t *target_scene;
    sigil::runtime::sync_data_t sync_data;
    
} private_state;

// Data for spellslot tracker columns
const char* column_names[] = {
    "Num/Lv","Lv.1", "Lv.2", "Lv.3", "Lv.4", "Lv.5", "Lv.6", "Lv.7", "Lv.8", "Lv.9",
};
const int columns_count = IM_ARRAYSIZE(column_names);
const int rows_count = 4;
ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_HighlightHoveredColumn;
ImGuiTableColumnFlags column_flags = ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed;
bool bools[columns_count * rows_count] = {}; // Dummy storage selection storage
int frozen_cols = 1;
int frozen_rows = 2;


static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct private_state_t *private_state = (struct private_state_t*)state->input;

    switch (key) {
    case 'd':
        sigil::utils::set_debug(255);
	    break;
    case 'k':
        sigil::env::fullscreen = true;
    case ARGP_KEY_ARG:
        return 0;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

#ifdef APP_USE_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    (void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
    fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
    return VK_FALSE;
}
#endif // APP_USE_VULKAN_DEBUG_REPORT




static void FrameRender(sigil::visor::vk_window_t* wd, ImDrawData* draw_data) {
    VkResult err;

    VkSemaphore image_acquired_semaphore  = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(sigil::visor::data->vk_dev, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        sigil::visor::data->rebuild_swap_chain = true;
        return;
    }

    sigil::visor::check_vk_result(err);

    sigil::visor::vk_framedata_t* fd = &wd->Frames[wd->FrameIndex];
    {
        err = vkWaitForFences(sigil::visor::data->vk_dev, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
        sigil::visor::check_vk_result(err);

        err = vkResetFences(sigil::visor::data->vk_dev, 1, &fd->Fence);
        sigil::visor::check_vk_result(err);
    }
    {
        err = vkResetCommandPool(sigil::visor::data->vk_dev, fd->CommandPool, 0);
        sigil::visor::check_vk_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        sigil::visor::check_vk_result(err);
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
        sigil::visor::check_vk_result(err);
        err = vkQueueSubmit(sigil::visor::data->vk_main_q.queue, 1, &info, fd->Fence);
        sigil::visor::check_vk_result(err);
    }
}

static void FramePresent(sigil::visor::vk_window_t* wd) {
    if (sigil::visor::data->rebuild_swap_chain) return;

    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(sigil::visor::data->vk_main_q.queue, &info);

    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        sigil::visor::data->rebuild_swap_chain = true;
        return;
    }

    sigil::visor::check_vk_result(err);
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount; // Now we can use the next set of semaphores
}

void toggle_bool(bool &val) {
    if (val) val = false;
    else val = true;
}

enum dice_types {
    D4, D6, D8, D10, D12, D20,
};

static const char* dice_type_to_str(dice_types type) {
    if (type == D4) return "d4";
    else if (type == D6) return "d6";
    else if (type == D8) return "d8";
    else if (type == D10) return "d10";
    else if (type == D12) return "d12";
    else if (type == D20) return "d20";
    return nullptr;
}

static void text_editor_window() {
    ImGui::Begin("Text Editor");
    ImGui::InputTextMultiline("Text", private_state.text_editor_buffer, sizeof(private_state.text_editor_buffer));
    ImGui::End();
}

static void dice_roller_window() {
    ImGui::Begin("Dice Roller");

    // D4
    if (ImGui::Button("-##1", ImVec2(16,16))) private_state.dice_roller_data.d4count--;
    ImGui::SameLine();
    ImGui::Text("%d d4", private_state.dice_roller_data.d4count);
    ImGui::SameLine();
    if (ImGui::Button("+##1", ImVec2(16,16))) private_state.dice_roller_data.d4count++;
    ImGui::SameLine();
    if (ImGui::Button("roll##1", ImVec2(32,16))) {
        private_state.dice_roller_data.d4roll = 0;
        for (int i = 0; i < private_state.dice_roller_data.d4count; i++) {
            int rnd = sigil::math::get_rs32(1, 4);
            private_state.dice_roller_data.d4roll += rnd;
        }
    }
    ImGui::SameLine();
    ImGui::Text("roll: %d", private_state.dice_roller_data.d4roll);

    // D6
    if (ImGui::Button("-##2", ImVec2(16,16))) private_state.dice_roller_data.d6count--;
    ImGui::SameLine();
    ImGui::Text("%d d6", private_state.dice_roller_data.d6count);
    ImGui::SameLine();
    if (ImGui::Button("+##2", ImVec2(16,16))) private_state.dice_roller_data.d6count++;
    ImGui::SameLine();
    if (ImGui::Button("roll##2", ImVec2(32,16))) {
        private_state.dice_roller_data.d6roll = 0;
        for (int i = 0; i < private_state.dice_roller_data.d6count; i++) {
            int rnd = sigil::math::get_rs32(1, 6);
            private_state.dice_roller_data.d6roll += rnd;
        }
    }
    ImGui::SameLine();
    ImGui::Text("roll: %d", private_state.dice_roller_data.d6roll);

    // D8
    if (ImGui::Button("-##3", ImVec2(16,16))) private_state.dice_roller_data.d8count--;
    ImGui::SameLine();
    ImGui::Text("%d d8", private_state.dice_roller_data.d8count);
    ImGui::SameLine();
    if (ImGui::Button("+##3", ImVec2(16,16))) private_state.dice_roller_data.d8count++;
    ImGui::SameLine();
    if (ImGui::Button("roll##3", ImVec2(32,16))) {
        private_state.dice_roller_data.d8roll = 0;
        for (int i = 0; i < private_state.dice_roller_data.d8count; i++) {
            int rnd = sigil::math::get_rs32(1, 8);
            private_state.dice_roller_data.d8roll += rnd;
        }
    }
    ImGui::SameLine();
    ImGui::Text("roll: %d", private_state.dice_roller_data.d8roll);

    // D10
    if (ImGui::Button("-##4", ImVec2(16,16))) private_state.dice_roller_data.d10count--;
    ImGui::SameLine();
    ImGui::Text("%d d10", private_state.dice_roller_data.d10count);
    ImGui::SameLine();
    if (ImGui::Button("+##4", ImVec2(16,16))) private_state.dice_roller_data.d10count++;
    ImGui::SameLine();
    if (ImGui::Button("roll##4", ImVec2(32,16))) {
        private_state.dice_roller_data.d10roll = 0;
        for (int i = 0; i < private_state.dice_roller_data.d10count; i++) {
            int rnd = sigil::math::get_rs32(1, 10);
            private_state.dice_roller_data.d10roll += rnd;
        }
    }
    ImGui::SameLine();
    ImGui::Text("roll: %d", private_state.dice_roller_data.d10roll);


    // D12
    if (ImGui::Button("-##5", ImVec2(16,16))) private_state.dice_roller_data.d12count--;
    ImGui::SameLine();
    ImGui::Text("%d d12", private_state.dice_roller_data.d12count);
    ImGui::SameLine();
    if (ImGui::Button("+##5", ImVec2(16,16))) private_state.dice_roller_data.d12count++;
    ImGui::SameLine();
    if (ImGui::Button("roll##5", ImVec2(32,16))) {
        private_state.dice_roller_data.d12roll = 0;
        for (int i = 0; i < private_state.dice_roller_data.d12count; i++) {
            int rnd = sigil::math::get_rs32(1, 12);
            private_state.dice_roller_data.d12roll += rnd;
        }
    }
    ImGui::SameLine();
    ImGui::Text("roll: %d", private_state.dice_roller_data.d12roll);


    // D20
    if (ImGui::Button("-##6", ImVec2(16,16))) private_state.dice_roller_data.d20count--;
    ImGui::SameLine();
    ImGui::Text("%d d20", private_state.dice_roller_data.d20count);
    ImGui::SameLine();
    if (ImGui::Button("+##6", ImVec2(16,16))) private_state.dice_roller_data.d20count++;
    ImGui::SameLine();
    if (ImGui::Button("roll##6", ImVec2(32,16))) {
        private_state.dice_roller_data.d20roll = 0;
        for (int i = 0; i < private_state.dice_roller_data.d20count; i++) {
            int rnd = sigil::math::get_rs32(1, 20);
            private_state.dice_roller_data.d20roll += rnd;
        }
    }
    ImGui::SameLine();
    ImGui::Text("roll: %d", private_state.dice_roller_data.d20roll);
    ImGui::End();
}

static void spellslot_tracker() {
    float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    ImGui::Begin("Spellslot tracker");
    if (ImGui::BeginTable("table_angled_headers", columns_count, table_flags, ImVec2(0.0f, TEXT_BASE_HEIGHT * 12))) {
            ImGui::TableSetupColumn(column_names[0], ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoReorder);
            for (int n = 1; n < columns_count; n++)
                ImGui::TableSetupColumn(column_names[n], column_flags);
            ImGui::TableSetupScrollFreeze(frozen_cols, frozen_rows);

            ImGui::TableAngledHeadersRow(); // Draw angled headers for all columns with the ImGuiTableColumnFlags_AngledHeader flag.
            ImGui::TableHeadersRow();       // Draw remaining headers and allow access to context-menu and other functions.
            for (int row = 0; row < rows_count; row++)
            {
                ImGui::PushID(row);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                //ImGui::Text("Track %d", row);
                for (int column = 1; column < columns_count; column++)
                    if (ImGui::TableSetColumnIndex(column))
                    {
                        ImGui::PushID(column);
                        ImGui::Checkbox("", &bools[row * columns_count + column]);
                        ImGui::PopID();
                    }
                ImGui::PopID();
            }
            ImGui::EndTable();
    }
    ImGui::End();
}

static void options_window() {
    ImGuiIO &io = ImGui::GetIO();
    ImGui::Begin("Options");
    
    if (ImGui::Checkbox("Vsync", &sigil::visor::data->vsync)) {
        sigil::visor::set_vk_present_mode(sigil::visor::data->vsync);
        assert(sigil::visor::data->min_image_count >= 2);
        ImGui_ImplVulkanH_CreateOrResizeWindow(sigil::visor::data->vk_inst, 
                    sigil::visor::data->vk_phy_active, sigil::visor::data->vk_dev, 
                    &sigil::visor::data->vk_window, sigil::visor::data->vk_main_q.family, 
                    sigil::visor::data->vk_allocators, sigil::visor::data->vk_window.Width, 
                    sigil::visor::data->vk_window.Height, sigil::visor::data->min_image_count);
    }
    ImGui::Checkbox("Limit to:", &sigil::env::limit_fps);
    ImGui::SameLine();
    ImGui::SliderInt("FPS", (int*)&private_state.sync_data.target_render_rate, 0, 200);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::ColorEdit3("Backgroud", (float*)&private_state.clear_color);
    ImGui::End();
}

static void output_window() {
    ImGuiIO &io = ImGui::GetIO();
    ImGui::Begin("Output");
    ImGui::Text("%s", private_state.output_window_buffer[0]);
    ImGui::Text("%s", private_state.output_window_buffer[1]);
    ImGui::Text("%s", private_state.output_window_buffer[2]);
    ImGui::Text("%s", private_state.output_window_buffer[3]);
    ImGui::End();
}

static void performance_popup() {
    ImGuiIO &io = ImGui::GetIO();
    sigil::memory::get_memstats(&memstats);
    ImGui::Begin("Performance", &private_state.show_performance_popup);

    //if (ImGui::Button("Close")) private_state.show_performance_popup = false;
    ImGui::Text("Frame time: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("Frames drawn: %lu, last frame ns %f", private_state.sync_data.iters, private_state.sync_data.delta_us);
    ImGui::Text("Program size (Pages/MB): %lu/%lukB", memstats.size, memstats.size * 4096);
    ImGui::Text("Text size (Pages/MB): %lu/%lukB", memstats.text, memstats.text * 4096);
    ImGui::Text("Shared memory (Pages/MB): %lu/%lukB", memstats.share, memstats.share * 4096);
    ImGui::Text("Data memory usage (Pages/MB): %lu/%lukB", memstats.data, memstats.data * 4096);
    ImGui::Text("Resident memory usage (Pages/MB): %lu/%lukB", memstats.resident, memstats.resident * 4096);

    ImGui::End();
}

static void asset_manager_window() {
    ImGui::Begin("Asset Manager");
    ImGui::Text("Num entities: %lu", private_state.target_scene->entities.size());
    if (ImGui::Button("Spawn 100k Entities")) {
        sigil::hat_engine::pull_rabbits(private_state.target_scene, 100000);
    }

    if (ImGui::Button("Spawn workqueue")) {
        private_state.err = sigil::runtime::spawn_runtime_workqueue();
        if (private_state.err) {
            ImGui::Text("Error code: %d", private_state.err);
        }
    }

    if (ImGui::CollapsingHeader("Modules")) {
        for (uint8_t i = 0; i < sigil::env::modules_registry.size(); i++) {
            sigil::modules::module_t *current = sigil::modules::peek(i);
            current->asset_mtx.lock();
            ImGui::Text("%s (%s)", current->name.c_str(), sigil::utils::asset_type_tostr(current->type));

            if (current->type == sigil::ASSET_WORKQUEUE) {
                sigil::workqueue_descriptor_t *temp = (sigil::workqueue_descriptor_t*)sigil::env::modules_registry.at(i)->data;
                temp->wq_mutex.lock();
                size_t num_wkitems = temp->wq.size();
                uint64_t num_processed = temp->items_processed;
                temp->wq_mutex.unlock();
                ImGui::Text("   Queued / Processed -> %lu / %lu", num_wkitems, num_processed);
            }
            current->asset_mtx.unlock();
        }
    }

    if (ImGui::CollapsingHeader("Events")) {
        for (uint8_t i = 0; i < sigil::env::events_registry.size(); i++) {
            sigil::events::event_t *current = sigil::events::peek(i);
            if (current == nullptr) continue;

            current->asset_mtx.lock();
            //ImGui::Text("%s (%s)", current->name.c_str(), sigil::utils::asset_type_tostr(current->type));

            if (current->type == sigil::ASSET_EVENT) {
                ImGui::Text("%s -> owner: %s, callbacks: %d, enabled: %s\n", current->name.c_str(),
                            current->owner ? ((sigil::modules::module_t*)(current->owner))->name.c_str() : "-none-",
                            (int)current->work.size(),
                            current->enabled ?  "yes" : "no");
            }
            current->asset_mtx.unlock();
        }
    }

    if (ImGui::CollapsingHeader("Hardware")) {
        for (int i = 0; i < sigil::visor::data->num_phys; i++) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(sigil::visor::data->vk_phy_all.at(i), &props);
            ImGui::Text("GPU: %s", props.deviceName);
        }
    }
    
    ImGui::End();
}

static void overview_window() {
    ImGui::Begin("Overview");
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    ImGui::Image(NULL, ImVec2{viewportPanelSize.x, viewportPanelSize.y});
    ImGui::End();
}

static void asset_editor_window() {
    ImGui::Begin("Asset Editor");

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    //ImGui::Image(NULL, ImVec2{viewportPanelSize.x, viewportPanelSize.y});
    ImGui::Text("Asset editor not yet available");
    ImGui::End();
}

static void draw_one_frame() {
    sigil::visor::vk_window_t *wd = &sigil::visor::data->vk_window;
    ImGuiIO &io = ImGui::GetIO();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();


    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Main Menu")) {
            if (ImGui::MenuItem("New project")) {}
            if (ImGui::MenuItem("Open project", "Ctrl+O")) {}
            if (ImGui::MenuItem("Save project", "Ctrl+S")) {}
            if (ImGui::MenuItem("Save project as..")) {}
            ImGui::Separator();
            ImGui::MenuItem("Options", NULL, &private_state.show_options_window);
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                sigil::events::event_t *shutdown = sigil::events::peek(sigil::runtime::SHUTDOWN);
                sigil::events::trigger(shutdown);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
            if (ImGui::MenuItem("Redo")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Show")) {
            ImGui::MenuItem("Performance", NULL, &private_state.show_performance_popup);
            ImGui::MenuItem("Output window", "Ctrl + O", &private_state.show_output_window);
            ImGui::MenuItem("Overview", NULL, &private_state.show_overview_window);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Assets")) {
            ImGui::MenuItem("Asset Manager", "Shift+A", &private_state.show_asset_manager_window);
            ImGui::MenuItem("Asset Editor", "Shift+A", &private_state.show_asset_editor_window);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Apps")) {
            ImGui::MenuItem("Spellslot Tracker", NULL, &private_state.show_spellslot_tracker);
            ImGui::MenuItem("Dice Roller", NULL, &private_state.show_dice_roller);
            ImGui::MenuItem("ImGui Demo", NULL ,&private_state.show_demo_window);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About Sigil")) {}
            if (ImGui::MenuItem("Shortcuts")) {}
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (private_state.show_text_editor) text_editor_window();
    if (private_state.show_asset_manager_window) asset_manager_window();
    if (private_state.show_asset_editor_window) asset_editor_window();
    if (private_state.show_overview_window) overview_window();
    if (private_state.show_performance_popup) performance_popup();
    if (private_state.show_options_window) options_window();
    if (private_state.show_demo_window) ImGui::ShowDemoWindow(&private_state.show_demo_window);
    if (private_state.show_spellslot_tracker) spellslot_tracker();
    if (private_state.show_dice_roller) dice_roller_window();
    if (private_state.show_output_window) output_window();
}

int main(int argc, char **argv) {
    sigil::memory::memstat_t memstats;
    VkResult vk_err;
    int err = 0;

    sigil::utils::set_hostname(argp_program_version);
    sigil::utils::set_iteration_delay(50);

    private_state.theme = private_state_t::DARKMODE;
    private_state.sync_data.target_render_rate = 0;
    private_state.sync_data.iters = 0;
    sigil::env::limit_fps = false;

    err = sigil::runtime::import();
    if (err) exit(err);

    argp_parse(&argp, argc, argv, 0, 0, &private_state);

    err = sigil::runtime::spawn_runtime_workqueue();
    if (err) exit(err);

    err = sigil::visor::import();
    if (err) exit(err);

    err = sigil::hid_driver::import();
    if (err) exit(err);

    err = sigil::ntt::import();
    if (err) exit(err);
    
    err = sigil::runtime::start();
    if (err) exit(err);
    
    private_state.target_scene = sigil::ntt::get_default_scene();
    

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable window docking
    //io.ConfigFlags |= ImGuiDockNodeFlags_PassthruCentralNode;
    //io.ConfigDockingTransparentPayload |= ImGuiDockNodeFlags_PassthruCentralNode;

    printf("main: Dear ImGui context initialized\n");
    if (private_state.theme == private_state_t::LIGHTMODE) ImGui::StyleColorsLight();
    else if (private_state.theme == private_state_t::DARKMODE) ImGui::StyleColorsDark();

    //sigil::utils::delay(500);
    printf("main: setting up renderer and hid backend\n");
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(sigil::visor::data->vk_window.glfw_window, true);
    sigil::visor::vk_init_info_t
    init_info = {};
    init_info.Instance = sigil::visor::data->vk_inst;
    init_info.PhysicalDevice = sigil::visor::data->vk_phy_active;
    init_info.Device = sigil::visor::data->vk_dev;
    init_info.QueueFamily = sigil::visor::data->vk_main_q.family;
    init_info.Queue = sigil::visor::data->vk_main_q.queue;
    init_info.PipelineCache = sigil::visor::data->vk_pipeline_cache;
    init_info.DescriptorPool = sigil::visor::data->vk_descriptor_pool;
    init_info.RenderPass = sigil::visor::data->vk_window.RenderPass;
    init_info.Subpass = 0;
    init_info.MinImageCount = sigil::visor::data->min_image_count;
    init_info.ImageCount = sigil::visor::data->vk_window.ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = sigil::visor::data->vk_allocators;
    init_info.CheckVkResultFn = sigil::visor::check_vk_result;
    printf("main: calling vk_init \n");
    sigil::visor::vk_init(&init_info);
    
    ImFont* font = io.Fonts->AddFontFromFileTTF("../assets/fonts/Inter-VariableFont_slnt,wght.ttf", 16.0f);
    assert(font != nullptr);
    font = io.Fonts->AddFontFromFileTTF("../assets/fonts/droid-sans-mono.ttf", 16.0f);
    assert(font != nullptr);

    while (!glfwWindowShouldClose(sigil::visor::data->vk_window.glfw_window)) {
        sigil::visor::vk_window_t *wd = &sigil::visor::data->vk_window;
        assert(wd != NULL);

        glfwPollEvents();

        // Resize swap chain?
        if (sigil::visor::data->rebuild_swap_chain) {
            int width, height;
            glfwGetFramebufferSize(sigil::visor::data->vk_window.glfw_window, &width, &height);
            if (width > 0 && height > 0) {
                ImGui_ImplVulkan_SetMinImageCount(sigil::visor::data->min_image_count);
                ImGui_ImplVulkanH_CreateOrResizeWindow(sigil::visor::data->vk_inst, 
                            sigil::visor::data->vk_phy_active, sigil::visor::data->vk_dev, 
                            &sigil::visor::data->vk_window, sigil::visor::data->vk_main_q. family, 
                            sigil::visor::data->vk_allocators, width, height, sigil::visor::data->min_image_count);
                sigil::visor::data->vk_window.FrameIndex = 0;
                sigil::visor::data->rebuild_swap_chain = false;
            }
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        draw_one_frame();


        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized) {
            wd->ClearValue.color.float32[0] = private_state.clear_color.x * private_state.clear_color.w;
            wd->ClearValue.color.float32[1] = private_state.clear_color.y * private_state.clear_color.w;
            wd->ClearValue.color.float32[2] = private_state.clear_color.z * private_state.clear_color.w;
            wd->ClearValue.color.float32[3] = private_state.clear_color.w;
            FrameRender(wd, draw_data);
            FramePresent(wd);
        }

        if (sigil::env::limit_fps) sigil::runtime::sync(&private_state.sync_data);
        else private_state.sync_data.iters++;
    }

    // Trigger cleanup procedures
    sigil::events::event_t *shutdown = sigil::events::peek(sigil::runtime::SHUTDOWN);
    sigil::events::trigger(shutdown);
    return 0;
}
