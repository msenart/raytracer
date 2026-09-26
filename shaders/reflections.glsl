#ifndef REFLECTIONS_GLSL
#define REFLECTIONS_GLSL

#include "random.glsl"

struct Material{
    uint id;
    vec4 color;
    bool emissive;
};

vec3 sampleLambertReflection(vec3 L, vec3 N){
    return normalize(normalize(N) + randomUnitVector());
}

vec3 sampleMetalReflection(vec3 L, vec3 N){
    return L - 2*dot(L,N)*N;
}

vec3 sampleReflection(vec3 L, vec3 N, Material material){
    if (material.id == 0) {return sampleLambertReflection(L,N);}
    if (material.id == 1) {return sampleMetalReflection(L,N);}

    else {return vec3(0,0,0);}
}

#endif
