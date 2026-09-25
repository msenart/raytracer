#pragma once
#include "Bufwrap.h"
#include "Shader.h"
struct Screen {
    VAO* vao;
    VBO* vbo;
    IBO* ibo;
    std::shared_ptr<Shader> sp;
    std::shared_ptr<Shader> sp2;
    unsigned int frame = 1;

    FBO* fbo1;
    FBO* fbo2;
    unsigned int fbo1_tex = 0;
    unsigned int fbo2_tex = 0;
    bool isFbo1 = true;

    Screen();

    void draw();

    /// @brief resets both textures values to the background values.
    void resetTextures();

    void windowResizeCallback(GLFWwindow* window, int width, int height) const;
};
