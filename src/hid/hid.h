// dear imgui: Platform Backend for GLFW
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)
// (Info: GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Mouse support. Can discriminate Mouse/TouchScreen/Pen (Windows only).
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass ImGuiKey values to all key functions e.g. ImGui::IsKeyPressed(ImGuiKey_Space). [Legacy GLFW_KEY_* values will also be supported unless IMGUI_DISABLE_OBSOLETE_KEYIO is set]
//  [X] Platform: Gamepad support. Enable with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange' (note: the resizing cursors requires GLFW 3.4+).

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#pragma once
#include "../imgui/imgui.h"      // IMGUI_IMPL_API
/*
    Module header for SigilVM, responsibilities:
    - Implementing default behavior for:
        - hardware buttons using GPIO
        - device provided by lcd4bit.h
        - uart port (forward msg to system module for parsing)
*/

#include "../core/utils.h"
#include "../core/modules.h"
#include "../core/runtime.h"
#include <cstdio>

#ifdef TARGET_8266
#include "driver/uart.h"
#endif /* TARGET_8266 */

#define HID_NAME "hid-driver"
#define HID_SIGNAL_BUTTON_RIGHT "hid-right"
#define HID_SIGNAL_BUTTON_LEFT "hid-left"
#define HID_SERIAL_SIZE 512
#define HID_SERIAL_BAUD_DEFAULT 74880
#define HID_BUTTON_RIGHT 13
#define HID_BUTTON_LEFT 12
#define HID_4BIT_REGISTER_SELECT 2
#define HID_4BIT_ENABLE_DEVICE 14
#define HID_4BIT_DS4 0
#define HID_4BIT_DS5 4
#define HID_4BIT_DS6 5
#define HID_4BIT_DS7 16
#define HID_POLLER_WAIT_TIME 5
#ifndef IMGUI_DISABLE

struct GLFWwindow;
struct GLFWmonitor;

IMGUI_IMPL_API bool     ImGui_ImplGlfw_InitForOpenGL(GLFWwindow* window, bool install_callbacks);
IMGUI_IMPL_API bool     ImGui_ImplGlfw_InitForVulkan(GLFWwindow* window, bool install_callbacks);
IMGUI_IMPL_API bool     ImGui_ImplGlfw_InitForOther(GLFWwindow* window, bool install_callbacks);
IMGUI_IMPL_API void     ImGui_ImplGlfw_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplGlfw_NewFrame();

// Emscripten related initialization phase methods
#ifdef __EMSCRIPTEN__
IMGUI_IMPL_API void     ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback(const char* canvas_selector);
#endif

// GLFW callbacks install
// - When calling Init with 'install_callbacks=true': ImGui_ImplGlfw_InstallCallbacks() is called. GLFW callbacks will be installed for you. They will chain-call user's previously installed callbacks, if any.
// - When calling Init with 'install_callbacks=false': GLFW callbacks won't be installed. You will need to call individual function yourself from your own GLFW callbacks.
IMGUI_IMPL_API void     ImGui_ImplGlfw_InstallCallbacks(GLFWwindow* window);
IMGUI_IMPL_API void     ImGui_ImplGlfw_RestoreCallbacks(GLFWwindow* window);

// GFLW callbacks options:
// - Set 'chain_for_all_windows=true' to enable chaining callbacks for all windows (including secondary viewports created by backends or by user)
IMGUI_IMPL_API void     ImGui_ImplGlfw_SetCallbacksChainForAllWindows(bool chain_for_all_windows);

// GLFW callbacks (individual callbacks to call yourself if you didn't install callbacks)
IMGUI_IMPL_API void     ImGui_ImplGlfw_WindowFocusCallback(GLFWwindow* window, int focused);        // Since 1.84
IMGUI_IMPL_API void     ImGui_ImplGlfw_CursorEnterCallback(GLFWwindow* window, int entered);        // Since 1.84
IMGUI_IMPL_API void     ImGui_ImplGlfw_CursorPosCallback(GLFWwindow* window, double x, double y);   // Since 1.87
IMGUI_IMPL_API void     ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
IMGUI_IMPL_API void     ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
IMGUI_IMPL_API void     ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
IMGUI_IMPL_API void     ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c);
IMGUI_IMPL_API void     ImGui_ImplGlfw_MonitorCallback(GLFWmonitor* monitor, int event);

namespace sigil {
    namespace hid_driver {
        struct hid_data_t {
            uint8_t data_buffer[HID_SERIAL_SIZE];
            
            sigil::modules::module_t *uart_worker;
            sigil::events::event_t *hid_right;
            sigil::events::event_t *hid_left;
        };

        extern sigil::modules::module_t *mod;
        extern sigil::hid_driver::hid_data_t *data;

        void uart_poller(void *context);
        void uart_poller_alt(void *context);
        void button_poller(void *context);
        int start_esp8266_uart(uint32_t new_baud_rate);
        int setup_button_events();
        int install_hid_lcd4bit();
        int lcd4bit_set_cursor(int x, int y);
        int lcd4bit_write_bytes(const char *bytes, int num_bytes);
        int init(void *ctx);
        int deinit(void *ctx);
        int import();
    }
}
#endif // #ifndef IMGUI_DISABLE
