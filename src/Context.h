#pragma once

#include "Camera.h"
#include "Screen.h"

/// @brief Context is a pack grouping objects that are important everywhere in the code.
/// Callable with static_cast<Context*>(glfwGetWindowUserPointer(w)).
struct Context {
    Camera camera;
    Screen screen;
};