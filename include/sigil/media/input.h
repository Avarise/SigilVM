#pragma once
#include <GLFW/glfw3.h>
#include "sigil/status.h"

// @tree-metadata: User input manager

namespace sigil {

struct input_mgr_t {
    bool dummy;
    status_t initialize(GLFWwindow *handle);
};

} // sigil