#include "Screen.h"
#include "Camera.h"
#include "Globals.h"

void Screen::draw() const {
    sp->use();
    vao->bind();

    // bind camera variables to the shader
    auto* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));

    auto translation = glm::mat4(1);
    sp->set("model", camera->model());
    sp->set("view", camera->view());
    sp->set("projection", camera->projection());

    glDisable(GL_CULL_FACE);
    glDrawElements(GL_TRIANGLES, 6  , GL_UNSIGNED_INT, nullptr);
}
