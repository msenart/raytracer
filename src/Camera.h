#pragma once

#include "OpenGLIncludes.h"

class Camera {
public:
    Camera(float fov_v, float window_width, float window_height, float near_v, float far_v);

    [[nodiscard]] glm::mat4 model() const;
    [[nodiscard]] glm::mat4 view() const;
    [[nodiscard]] glm::mat4 projection() const;

    void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    void processKeyboard(GLFWwindow* window, float deltaTime);

    glm::vec3 position{0.0f, 0.0f, 5.0f};
    glm::vec3 center{0.0f, 0.0f, 0.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};

    float fov;
    float aspectRatio;
    float nearPlane;
    float farPlane;

    float speed = 10.0f;
    float yaw = -90.0f;
    float pitch = 0.0f;

    bool can_move = false;
    bool ismoving = false;
    float lastX;
    float lastY;
    bool justAfterQuittingUI;
};
