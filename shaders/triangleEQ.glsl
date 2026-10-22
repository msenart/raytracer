#ifndef TRIANGLEEQ_GLSL
#define TRIANGLEEQ_GLSL

#include "hitproperties.glsl"
#include "vertices.glsl"
#include "transform.glsl"
// Möller-Trumbore algorithm for triangle intersection

vec3 triangleNormal(uint i0, uint i1, uint i2, uint transform_index, vec2 uv, bool interpolation) {
    Vertex v0 = vertices[i0];
    Vertex v1 = vertices[i1];
    Vertex v2 = vertices[i2];

    mat4 M = transforms[transform_index];

    vec3 p0 = (M * vec4(v0.position, 1.0)).xyz;
    vec3 p1 = (M * vec4(v1.position, 1.0)).xyz;
    vec3 p2 = (M * vec4(v2.position, 1.0)).xyz;

    vec3 ng = normalize(cross(p1 - p0, p2 - p0));
    if (determinant(mat3(M)) < 0.0) ng = -ng;
    if (!interpolation) return ng;

    mat3 normalMatrix = transpose(inverse(mat3(M)));
    float w = 1.0 - uv.x - uv.y;
    vec3 n = normalize(normalMatrix * (w * v0.normal + uv.x * v1.normal + uv.y * v2.normal));

    if (dot(n, ng) < 0.0) n = -n;
    return n;
}

HitProperties isRayIntersectingTriangle(Ray r, uint i0, uint i1, uint i2, uint transform_index, bool normalInterpolation, uint argmin_mesh_type, uint argmin_mesh){
    mat4 transform = transforms[transform_index];
    vec3 v0 = vec3(transform*vec4(vertices[i0].position, 1.f));
    vec3 v1 = vec3(transform*vec4(vertices[i1].position, 1.f));
    vec3 v2 = vec3(transform*vec4(vertices[i2].position, 1.f));

    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;

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
        return HitProperties(true, argmin_mesh_type, argmin_mesh, t, vec2(u, v), triangleNormal(i0, i1, i2, transform_index, vec2(u, v), normalInterpolation));
    }
    else
    return HitProperties(false, argmin_mesh_type, argmin_mesh, 0, vec2(0), vec3(0));
}

#endif