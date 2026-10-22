#pragma once

#include "OpenGLIncludes.h"

class Shader;

class Camera {
public:
    Camera(float window_width, float window_height, float speed, float fov_v, float near_v, float far_v);

    [[nodiscard]] glm::mat4 view() const;
    [[nodiscard]] glm::mat4 projection() const;

    /// @brief World space direction of the ray going through a point of the screen.
    /// @param ndcX x in normalized device coordinates, [-1, 1] from left to right.
    /// @param ndcY y in normalized device coordinates, [-1, 1] from bottom to top.
    [[nodiscard]] glm::vec3 screenRay(float ndcX, float ndcY) const;

    void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    void processKeyboard(GLFWwindow* window, float deltaTime);
    void setShaderValues(const std::shared_ptr<Shader>& sp) const;

    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 center{0.0f, 0.0f, 1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};

    float fov;
    float aspectRatio;
    float nearPlane;
    float farPlane;

    float speed = 10.0f;
    float yaw = -90.0f;
    float pitch = 0.0f;

    float apertureRadius = glm::radians(10.0f);
    float focusDistance = 5.f;

    bool can_move = false;
    bool is_moving = false;
    float lastX;
    float lastY;
    bool justAfterQuittingUI;
};
