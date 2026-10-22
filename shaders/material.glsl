#ifndef MATERIAL_GLSL
#define MATERIAL_GLSL

#include "params.glsl"

struct Material{
    float albedo[N_BINS];
    uint id;
    float metal_fuzz;
    float T;
    float intensity;
    float[N_BINS] n;
    float[N_BINS] k;
};

// All the materials of the scene. The last one is the air.
layout(std430, binding = 3) readonly buffer MaterialBuffer {
    Material materials[];
};

#endif