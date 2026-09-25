#include "Context.h"
#include "Globals.h"
#include "Parameters.h"

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
    sp2 = std::make_shared<Shader>(Shader::fromFiles("fbo_render.vert","fbo_render.frag"));

    fbo1 = new FBO();
    fbo2 = new FBO();

    fbo1->bind();
    glGenTextures(1, &fbo1_tex);
    glBindTexture(GL_TEXTURE_2D, fbo1_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    fbo1->attachTexture2D(fbo1_tex);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr<<"ERROR::FRAMEBUFFER:: Framebuffer is not complete"<<std::endl;
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    fbo2->bind();
    glGenTextures(1, &fbo2_tex);
    glBindTexture(GL_TEXTURE_2D, fbo2_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    fbo2->attachTexture2D(fbo2_tex);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr<<"ERROR::FRAMEBUFFER:: Framebuffer is not complete"<<std::endl;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    FBO::unbind();
}

void Screen::draw() {
    sp->bind();
    sp->set("frame",frame);
    vao->bind();
    // choosing which fbo to render
    auto read_fbo = isFbo1 ? fbo1 : fbo2;
    auto write_fbo = isFbo1 ? fbo2 : fbo1;
    auto read_tex = isFbo1 ? fbo1_tex : fbo2_tex;
    auto write_tex = isFbo1 ? fbo2_tex : fbo1_tex;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,read_tex);
    sp->set("readTex",0);

    write_fbo->bind();
    isFbo1 = !isFbo1;

    // bind camera variables to the shader
    auto context = static_cast<Context*>(glfwGetWindowUserPointer(window));
    context->camera.setShaderValues(sp);

    glClearColor(1.00f, 0.49f, 0.04f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDrawElements(GL_TRIANGLES, 6  , GL_UNSIGNED_INT, nullptr);
    FBO::unbind();
    sp->unbind();

    sp2->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,write_tex);
    sp->set("renderTex",0);
    glClearColor(1.00f, 0.49f, 0.04f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDrawElements(GL_TRIANGLES, 6  , GL_UNSIGNED_INT, nullptr);

    frame+=1;
}

void Screen::resetTextures() {
    glBindTexture(GL_TEXTURE_2D, fbo1_tex);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, fbo1_tex);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    frame = 1;
}

void Screen::windowResizeCallback(GLFWwindow *window, int width, int height) const {
    glBindTexture(GL_TEXTURE_2D, fbo1_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, fbo2_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
}
