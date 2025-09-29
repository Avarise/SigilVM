#include <sigil/render/device.h>
#include <sigil/status.h>
#include <string>

// namespace sigil {

// status_t window_t::initialize(std::string title) {
//     status_t status = VM_OK;
    
//     (void)title;

//     glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//     this->handle = glfwCreateWindow(this->width, this->height, this->title.c_str(), nullptr, nullptr);
    
//     if (!glfwVulkanSupported()) {
//         printf("GLFW: Vulkan Not Supported\n");
        
//         status = VM_NOT_SUPPORTED;
//         return status;
//     }


    
//     return status;
// }

// } // namespace sigil