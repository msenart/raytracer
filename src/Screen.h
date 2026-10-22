#pragma once
#include "Bufwrap.h"
#include "Shader.h"

struct RaytracerParams {
    unsigned int max_iter             = 50;
    float        max_distance         = 2000.f;
    float        background_intensity = 1.f;
    float        eps                  = 10E-5f;
    unsigned int multisampling_size   = 4;
    unsigned int random_sampling_size = 1;
    float        lambda_min           = .4f;
    float        lambda_max           = .8f;
    float        attenuation_coef     = 10E-2f;
    float        pi                   = 3.141592653589793f;

    void upload(const std::shared_ptr<Shader>& cs) const {
        cs->set("MAX_ITER",             max_iter);
        cs->set("MAX_DISTANCE",         max_distance);
        cs->set("BACKGROUND_INTENSITY", background_intensity);
        cs->set("eps",                  eps);
        cs->set("MULTISAMPLING_SIZE",   multisampling_size);
        cs->set("RANDOM_SAMPLING_SIZE", random_sampling_size);
        cs->set("LAMBDA_MIN",           lambda_min);
        cs->set("LAMBDA_MAX",           lambda_max);
        cs->set("ATTENUATION_COEF",     attenuation_coef);
        cs->set("PI",                   pi);
    }
};

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

    RaytracerParams raytracer_params;

    bool showUI = true;

    Screen();

    void draw();

    /// @brief resets both textures values to the background values.
    void resetTextures();

    void windowResizeCallback(GLFWwindow* window_v, int width, int height);
};
