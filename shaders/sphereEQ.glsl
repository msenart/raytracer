#ifndef SPHEREEQ_GLSL
#define SPHEREEQ_GLSL

#include "hitproperties.glsl"
#include "ray.glsl"
#include "params.glsl"

struct Sphere {
    vec3  pos;
    float radius;
    uint  material_id;
    float _pad0[3];
};

// Sphere intersection algorithm

vec3 sphereNormal(Sphere s, vec3 pos){
    return normalize(pos - s.pos);
}

HitProperties isRayIntersectingSphere(Ray r, Sphere s){
    float a = dot(r.d, r.d);
    float b = 2*dot(r.d, r.o - s.pos);
    float c = dot(r.o - s.pos, r.o - s.pos) - s.radius*s.radius;
    float delta = b*b - 4 * a * c;

    if (delta < 0){ // if there is no solution
        return HitProperties(false, 0, vec2(0), vec3(0));
    }
    // we have to find the smallest positive solution, otherwise, no intersection.
    if ((-b-sqrt(delta))/(2.f*a) > eps){
        float sol = (-b-sqrt(delta))/(2.f*a);
        return HitProperties(true, sol, vec2(0), sphereNormal(s, r.o + sol*r.d));
    }
    else if ((-b+sqrt(delta))/(2.f*a) > eps){
        float sol = (-b+sqrt(delta))/(2.f*a);
        return HitProperties(true, sol, vec2(0), sphereNormal(s, r.o + sol*r.d));
    }
    else {
        return HitProperties(false, 0, vec2(0), vec3(0));
    }
}

layout(std430, binding = 2) readonly buffer SpheresBuffer {
    Sphere spheres[];
};

#endif