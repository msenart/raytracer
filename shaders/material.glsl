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

float air_albedo[N_BINS]= {
1,1,1,1,
1,1,1,1,
1,1,1,1,
1,1,1,1};

float air_n[N_BINS] = {
1,1,1,1,
1,1,1,1,
1,1,1,1,
1,1,1,1};

float air_k[N_BINS] = {
0,0,0,0,
0,0,0,0,
0,0,0,0,
0,0,0,0 };

const Material airMaterial = Material(
        air_albedo,
        3,
        0.f,
        293.f,
        0.f,
        air_n,
        air_k
);

#endif