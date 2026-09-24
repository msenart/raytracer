#pragma once
#include "Bufwrap.h"
#include "Shader.h"
struct Screen {
    VAO* vao;
    VBO* vbo;
    IBO* ibo;
    std::shared_ptr<ShaderProgram> sp;

    Screen() {
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
        sp = ShaderProgram::genShaderProgram("../shaders/test.vert", "../shaders/test.frag");
    }

    void draw() const;
};
