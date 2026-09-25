#pragma once
#include "Bufwrap.h"
#include "Shader.h"
struct Screen {
    VAO* vao;
    VBO* vbo;
    IBO* ibo;
    std::shared_ptr<Shader> sp;
    unsigned int frame = 0;

    Screen();

    void draw();
};
