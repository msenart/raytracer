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
    vec3 lam0;
    float _pad0;
    vec3 Gam;
    float _pad1;
};

#endif