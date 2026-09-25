#ifndef RAY_GLSL
#define RAY_GLSL

struct Ray {
    vec3 o;
    vec3 d;
    vec4 throughput;
};

#endif
