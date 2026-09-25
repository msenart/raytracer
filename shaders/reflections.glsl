#ifndef REFLECTIONS_GLSL
#define REFLECTIONS_GLSL

#include "random.glsl"

vec3 sampleLambertReflection(vec3 L, vec3 N){
    return normalize(normalize(N) + randomUnitVector());
}

#endif
