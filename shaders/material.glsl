#ifndef MATERIAL_GLSL
#define MATERIAL_GLSL

struct Material{
    float reflectance[N_BINS];
    uint id;
    float metal_fuzz;
    float T;
    float _pad0;
    vec3 B;
    float _pad1;
    vec3 C;
    float _pad2;
};

#endif