#include "Screen.h"
#include "Camera.h"
#include "Globals.h"

void Screen::draw() const {
    sp->bind();
    vao->bind();

    // bind camera variables to the shader
    auto* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));

    camera->setShaderValues(sp);

    glDisable(GL_CULL_FACE);
    glDrawElements(GL_TRIANGLES, 6  , GL_UNSIGNED_INT, nullptr);
}
