#ifndef SPHEREEQ_GLSL
#define SPHEREEQ_GLSL

#include "hitproperties.glsl"
#include "ray.glsl"
#include "sphereSDF.glsl"
// Sphere intersection algorithm

vec3 sphereNormal(Sphere s, vec3 pos){
    return normalize(pos - s.pos);
}

HitProperties isRayIntersectingSphere(Ray r, Sphere s){
    float a = dot(r.d, r.d);
    float b = 2*dot(r.d, r.o - s.pos);
    float c = dot(r.o - s.pos, r.o - s.pos) - s.radius*s.radius;
    float delta = b*b - 4 * a * c;

    if (delta < eps){ // if there si no solution
        return HitProperties(false, 0, vec2(0), vec3(0));
    }
    // we have to find the smallest positive solution, otherwise, no intersection.
    if ((-b-sqrt(delta))/2*a > eps){
        float sol = (-b-sqrt(delta))/2*a;
        return HitProperties(true, (-b-sqrt(delta))/2*a, vec2(0), sphereNormal(s, r.o + sol*r.d));
    }
    else if ((-b+sqrt(delta))/2*a > eps){
        float sol = (-b+sqrt(delta))/2*a;
        return HitProperties(true, (-b+sqrt(delta))/2*a, vec2(0), sphereNormal(s, r.o + sol*r.d));
    }
    else {
        return HitProperties(false, 0, vec2(0), vec3(0));
    }
}

#endif