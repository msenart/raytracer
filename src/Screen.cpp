#include "Screen.h"
#include "Camera.h"
#include "Globals.h"

Screen::Screen() {
    vao = new VAO();
    vao->bind();
    vbo = new VBO();
    ibo = new IBO();
    vao->bind();

    std::vector<float> screen = {
        -1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, -1.0f,
    };
    vbo->setData(screen.size()*sizeof(float),screen.data(),GL_STATIC_DRAW);
    VertexAttribute va = {
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        2*sizeof(float),
        nullptr,
    };
    std::vector<unsigned int> indices = {
        2,1,0,3,2,0
    };
    ibo->setData(indices.size()*sizeof(unsigned int),indices.data(), GL_STATIC_DRAW);
    vao->addVBO(*vbo,{va});
    vao->unbind();
    sp = std::make_shared<Shader>(Shader::fromFiles("test.vert","test.frag"));
}

void Screen::draw() {
    sp->bind();

    sp->set("frame",frame);

    vao->bind();

    // bind camera variables to the shader
    auto* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));

    camera->setShaderValues(sp);

    glDisable(GL_CULL_FACE);
    glDrawElements(GL_TRIANGLES, 6  , GL_UNSIGNED_INT, nullptr);

    frame+=1;
}
