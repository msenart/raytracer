#pragma once
#include "Bufwrap.h"
#include "Shader.h"
struct Screen {
    VAO* vao;
    VBO* vbo;
    IBO* ibo;
    std::shared_ptr<Shader> sp;
    std::shared_ptr<Shader> sp2;
    std::shared_ptr<Shader> cs;
    unsigned int frame = 1;

    unsigned int tex1 = 0;
    unsigned int tex2 = 0;
    bool isTex1 = true;

    Screen();

    void draw();

    /// @brief resets both textures values to the background values.
    void resetTextures();

    void windowResizeCallback(GLFWwindow* window_v, int width, int height);
};
