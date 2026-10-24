#ifndef TRIANGLEEQ_GLSL
#define TRIANGLEEQ_GLSL

#include "hitproperties.glsl"
#include "vertices.glsl"

// Only computed for the closest hit of a ray, once the traversal is over.
vec3 triangleNormal(uint i0, uint i1, uint i2, vec2 uv, bool interpolation) {
    Vertex v0 = vertices[i0];
    Vertex v1 = vertices[i1];
    Vertex v2 = vertices[i2];

    vec3 ng = normalize(cross(v1.position - v0.position, v2.position - v0.position));
    if (!interpolation) return ng;

    float w = 1.0 - uv.x - uv.y;
    vec3 n = normalize(w * v0.normal + uv.x * v1.normal + uv.y * v2.normal);

    if (dot(n, ng) < 0.0) n = -n;
    return n;
}

// The normal of the returned hit is not set, see triangleNormal.
HitProperties isRayIntersectingTriangle(Ray r, uint i0, uint i1, uint i2, uint argmin_mesh_type, uint argmin_mesh){
    vec3 v0 = vertices[i0].position;
    vec3 e1 = vertices[i1].position - v0;
    vec3 e2 = vertices[i2].position - v0;

    vec3 h = cross(r.d, e2);
    float a = dot(h, e1);
    if (abs(a) < eps){
        return HitProperties(false, argmin_mesh_type, argmin_mesh, 0, vec2(0), vec3(0));
    }
    float f = 1.f/a;
    vec3 s = r.o - v0;
    float u = f * dot(h,s);
    if (u < 0.0 || u > 1.0){
        return HitProperties(false, argmin_mesh_type, argmin_mesh, 0, vec2(0), vec3(0));
    }
    vec3 q = cross(s,e1);
    float v = f * dot(r.d,q);
    if (v < 0.0 || u + v > 1.0){
        return HitProperties(false, argmin_mesh_type, argmin_mesh, 0, vec2(0), vec3(0));
    }
    float t = f * dot(q, e2);
    if (t > eps)
    {
        return HitProperties(true, argmin_mesh_type, argmin_mesh, t, vec2(u, v), vec3(0));
    }
    else
    return HitProperties(false, argmin_mesh_type, argmin_mesh, 0, vec2(0), vec3(0));
}

#endif