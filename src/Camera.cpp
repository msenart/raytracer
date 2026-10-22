#include "Camera.h"
#include "Shader.h"
#include "Context.h"
#include "Screen.h"

Camera::Camera(float window_width, float window_height, float speed, float fov_v, float near_v, float far_v) : speed(speed), fov(fov_v), aspectRatio(window_width/window_height),nearPlane(near_v),farPlane(far_v){
lastX = 0; lastY = 0;justAfterQuittingUI = true;
}

glm::mat4 Camera::view() const { return glm::lookAt(position, center, up); }

glm::mat4 Camera::projection() const { return glm::perspective(fov, aspectRatio, nearPlane, farPlane); }

glm::vec3 Camera::screenRay(float ndcX, float ndcY) const {
    const glm::vec4 p = glm::inverse(projection() * view()) * glm::vec4(ndcX, ndcY, 1.f, 1.f);
    return glm::normalize(glm::vec3(p) / p.w - position);
}

void Camera::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (justAfterQuittingUI) {
        lastX = xpos;
        lastY = ypos;
        justAfterQuittingUI = false;
    }

    if (!can_move) {
        return;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    if (glm::abs(xoffset) > 0 && glm::abs(yoffset) > 0) {
        is_moving = true;
    }
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    // Clamp pitch
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // Compute forward vector
    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    center = position + glm::normalize(dir);

    if (is_moving) {
        auto context = static_cast<Context*>(glfwGetWindowUserPointer(window));
        context->screen.resetTextures();
    }
    is_moving = false;
}

void Camera::processKeyboard(GLFWwindow* window, float deltaTime) {
    if (!can_move) {
        return;
    }
    float velocity = speed * deltaTime;
    glm::vec3 direction = normalize((center-position));
    glm::vec3 flatDirection = normalize(((center-position) - up*glm::dot((center-position), up)));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {position += flatDirection * velocity; is_moving = true;}
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {position -= flatDirection * velocity; is_moving = true;}
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {position -= glm::normalize(glm::cross(flatDirection, up)) * velocity; is_moving = true;}
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {position += glm::normalize(glm::cross(flatDirection, up)) * velocity; is_moving = true;}
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {position += up * velocity; is_moving = true;}
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {position -= up * velocity ; is_moving = true;}
    center = position + direction;

    if (is_moving) {
        auto context = static_cast<Context*>(glfwGetWindowUserPointer(window));
        context->screen.resetTextures();
    }
    is_moving = false;
}

void Camera::setShaderValues(const std::shared_ptr<Shader> &sp) const {
    sp->bind();
    sp->set("camera.position", position);
    sp->set("camera.up", up);
    sp->set("camera.aspectRatio", aspectRatio);
    sp->set("camera.fov",fov);
    sp->set("camera.nearPlane", nearPlane);
    sp->set("camera.farPlane", farPlane);
    sp->set("camera.view", view());
    sp->set("camera.inverseView", glm::inverse(view()));
    sp->set("camera.projection", projection());
    sp->set("camera.apertureRadius", apertureRadius);
    sp->set("camera.focusDistance", focusDistance);
}
