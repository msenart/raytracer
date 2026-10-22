#pragma once

#include "Camera.h"
#include "ObjectManager.h"
#include "Screen.h"
#include "ObjectManager.h"

/// @brief Context is a pack grouping objects that are important everywhere in the code.
/// Callable with static_cast<Context*>(glfwGetWindowUserPointer(w)).
struct Context {
    Camera camera;
    Screen screen;
    ObjectManager objectManager;

    /// @brief Switches between rasterization and raytracing. Entering raytracing computes the BVH of the current scene.
    void setRenderMode(RenderMode mode) {
        if (mode == screen.mode) return;
        if (mode == RenderMode::Raytracing) {
            objectManager.rebuildAccelerationStructure();
        }
        screen.mode = mode;
        screen.resetTextures();
    }

    /// @brief Selects the object under the cursor (or at the center of the screen when the cursor is captured).
    void pickAtCursor(GLFWwindow* w) {
        float ndcX = 0.f, ndcY = 0.f;
        if (glfwGetInputMode(w, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
            double x, y;
            int width, height;
            glfwGetCursorPos(w, &x, &y);
            glfwGetWindowSize(w, &width, &height);
            ndcX = static_cast<float>(2.0 * x / width - 1.0);
            ndcY = static_cast<float>(1.0 - 2.0 * y / height);
        }
        objectManager.pick(camera.position, camera.screenRay(ndcX, ndcY));
    }
};
