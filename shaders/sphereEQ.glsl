#ifndef SPHEREEQ_GLSL
#define SPHEREEQ_GLSL

#include "hitproperties.glsl"
#include "ray.glsl"
#include "params.glsl"
#include "transform.glsl"

struct Sphere {
    uint transform_index;
    float radius;
    uint  material_id;
    float _pad0;
};

vec3 position(Sphere s){
    return vec3(
            transforms[s.transform_index][3][0],
            transforms[s.transform_index][3][1],
            transforms[s.transform_index][3][2]
    );
}

// Sphere intersection algorithm

vec3 sphereNormal(Sphere s, vec3 pos){
    return normalize(pos - position(s));
}

HitProperties isRayIntersectingSphere(Ray r, Sphere s, uint argmin_mesh_type, uint argmin_mesh){
    float a = dot(r.d, r.d);
    float b = 2*dot(r.d, r.o - position(s));
    float c = dot(r.o - position(s), r.o - position(s)) - s.radius*s.radius;
    float delta = b*b - 4 * a * c;

    if (delta < 0){ // if there is no solution
        return HitProperties(false, argmin_mesh_type, argmin_mesh, 0, vec2(0), vec3(0));
    }
    // we have to find the smallest positive solution, otherwise, no intersection.
    if ((-b-sqrt(delta))/(2.f*a) > eps){
        float sol = (-b-sqrt(delta))/(2.f*a);
        return HitProperties(true, argmin_mesh_type, argmin_mesh, sol, vec2(0), sphereNormal(s, r.o + sol*r.d));
    }
    else if ((-b+sqrt(delta))/(2.f*a) > eps){
        float sol = (-b+sqrt(delta))/(2.f*a);
        return HitProperties(true, argmin_mesh_type, argmin_mesh, sol, vec2(0), sphereNormal(s, r.o + sol*r.d));
    }
    else {
        return HitProperties(false, argmin_mesh_type, argmin_mesh, 0, vec2(0), vec3(0));
    }
}

layout(std430, binding = 2) readonly buffer SpheresBuffer {
    Sphere spheres[];
};

#endif