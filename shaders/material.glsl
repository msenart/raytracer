#ifndef MATERIAL_GLSL
#define MATERIAL_GLSL

struct Material{
    float albedo[N_BINS];
    uint id;
    float metal_fuzz;
    float T;
    float intensity;
    vec3 B;
    float eta_test;
    vec3 C;
    float _pad2;
};

#endif