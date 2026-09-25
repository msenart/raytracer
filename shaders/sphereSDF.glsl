#ifndef SPHERESDF_GLSL
#define SPHERESDF_GLSL

#include "params.glsl"

struct Sphere{
    vec3 pos;
    float radius;
    vec4 color;
    bool emissive;
};

float distanceToSphere(vec3 x, Sphere s){
    return length(x-s.pos) - s.radius;
}

vec3 normalToSphere(vec3 p, vec3 d, in Sphere s) {
    vec3 outward_normal = normalize(
            vec3(
                    distanceToSphere(p + vec3(eps,0,0), s) - distanceToSphere(p - vec3(eps,0,0), s),
                    distanceToSphere(p + vec3(0,eps,0), s) - distanceToSphere(p - vec3(0,eps,0), s),
                    distanceToSphere(p + vec3(0,0,eps), s) - distanceToSphere(p - vec3(0,0,eps), s)
            )
    );

    return dot(outward_normal,d) > eps ? -outward_normal : outward_normal;
}

#endif