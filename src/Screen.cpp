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
    sp = std::make_shared<Shader>(Shader::fromFiles("test.vert","meshtracing.frag"));
    sp2 = std::make_shared<Shader>(Shader::fromFiles("fbo_render.vert","fbo_render.frag"));
    cs = std::make_shared<Shader>(Shader::computeFile("meshtracing.comp"));

    glGenTextures(1, &tex1);
    glBindTexture(GL_TEXTURE_2D, tex1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr<<"ERROR::FRAMEBUFFER:: Framebuffer is not complete"<<std::endl;
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &tex2);
    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr<<"ERROR::FRAMEBUFFER:: Framebuffer is not complete"<<std::endl;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    FBO::unbind();
}

void Screen::draw() {
    cs->bind();
    cs->set("frame",frame);
    cs->set("windowSize", glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT));
    vao->bind();
    // choosing which fbo to render
    auto read_tex = isTex1 ? tex1 : tex2;
    auto write_tex = isTex1 ? tex2 : tex1;

    // attaching reading buffer
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,read_tex);
    cs->set("readTex",0);

    isTex1 = !isTex1;

    // bind camera variables to the shader
    auto context = static_cast<Context*>(glfwGetWindowUserPointer(window));
    context->camera.setShaderValues(cs);

    // attaching writing buffer to the compute shader
    glBindImageTexture(0, write_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute((WINDOW_WIDTH + 7) / 8, (WINDOW_HEIGHT + 7) / 8, 1);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

    FBO::unbind();
    cs->unbind();

    sp2->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,write_tex);
    sp2->set("renderTex",0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDrawElements(GL_TRIANGLES, 6  , GL_UNSIGNED_INT, nullptr);

    frame+=1;
}

void Screen::resetTextures() {
    glBindTexture(GL_TEXTURE_2D, tex1);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, tex1);
    glClear(GL_COLOR_BUFFER_BIT);
    frame = 1;
}

void Screen::windowResizeCallback(GLFWwindow *window_v, int width, int height) {
    glBindTexture(GL_TEXTURE_2D, tex1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    frame = 1;
}
