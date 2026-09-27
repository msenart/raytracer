#version 450 core

#include "camera.glsl"
#include "sphereSDF.glsl"
#include "params.glsl"
#include "ray.glsl"
#include "reflections.glsl"

uniform sampler2D readTex;

out vec4 fragColor;
in vec2 uvScreen;

uniform Camera camera;
uniform vec2 windowSize;
uniform uint frame;

Sphere[MAX_MESH] spheres;
uint spheres_size = 0;

// Vertices SSBO

struct Vertex{
    vec3 position;  float _pad0;
    vec3 normal;    float _pad1;
    vec4 tangent;
    vec2 uv;        vec2 _pad2;
};

layout(std430, binding = 0) readonly buffer VertexBuffer {
    Vertex vertices[];
};

// Indices SSBO

struct IndexAndMeshID{
    uint index;
    uint mesh_id;
    uint _pad0;
    uint _pad1;
};

layout(std430, binding = 1) readonly buffer IndicesBuffer {
    IndexAndMeshID indices[];
};

// Möller-Trumbore algorithm

struct HitProperties{
    bool hasHit;
    float t;
    float u;
    float v;
};

HitProperties isRayIntersectingTriangle(Ray r, uint i0, uint i1, uint i2){
    vec3 v0 = vertices[i0].position;
    vec3 v1 = vertices[i1].position;
    vec3 v2 = vertices[i2].position;

    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;

    vec3 h = cross(r.d, e2);
    float a = dot(h, e1);
    if (abs(a) < eps){
        return HitProperties(false, 0, 0, 0);
    }
    float f = 1.f/a;
    vec3 s = r.o - v0;
    float u = f * dot(h,s);
    if (u < 0.0 || u > 1.0){
        return HitProperties(false, 0, 0, 0);
    }
    vec3 q = cross(s,e1);
    float v = f * dot(r.d,q);
    if (v < 0.0 || u + v > 1.0){
        return HitProperties(false, 0, 0, 0);
    }

    float t = f * dot(q, e2);
    if (t > eps)
    {
        return HitProperties(true, t, u, v);
    }
    else
    return HitProperties(false, 0, 0, 0);
}

// Core raytracing mechanic
Material airMaterial = Material(2,vec4(1,1,1,1),false,0.f, 1.f);

vec4 Trace(Ray r) {
    float min_distance = 10000.0;
    uint argmin_mesh = MAX_MESH;
    uint argmin_triangle = 0; // you'll want this for shading

    for (uint i = 0; i < indices.length(); i += 3) {
        HitProperties hp = isRayIntersectingTriangle(r, indices[i].index, indices[i+1].index, indices[i+2].index);
        if (hp.hasHit && hp.t < min_distance) {
            min_distance = hp.t;
            argmin_mesh = indices[i].mesh_id;
            argmin_triangle = i; // remember which triangle, for shading later
        }
    }
    if (argmin_mesh < MAX_MESH) {
        return vec4(0.7f, 0, 0, 1); // placeholder — see below
    }
    return BACKGROUND_COLOR; // or background color
}

// No blurring

Ray setWorldRay(vec3 d){
    mat3 camToWorld = mat3(inverse(camera.view));
    vec3 worldRayDir = normalize(camToWorld * d);
    return Ray(camera.position, worldRayDir, vec4(1, 1, 1, 1), MAX_MESH);
}

// Focus blurring

Ray setWorldRayFocusBlur(vec3 d){
    vec3 focusPoint = d*camera.focusDistance;

    vec3 lensOffset = vec3(camera.apertureRadius * randomUnitVectorInDisk(), 0.0);

    vec3 newOrigin = lensOffset;
    vec3 newDir = normalize(focusPoint - lensOffset);

    mat3 camToWorld = mat3(inverse(camera.view));
    vec3 worldOrigin = camera.position + camToWorld * newOrigin;
    vec3 worldDirection = normalize(camToWorld * newDir);
    return Ray(worldOrigin, worldDirection, vec4(1, 1, 1, 1), MAX_MESH);
}

// Basic raytracing
vec4 basicRayTracing(){
    vec2 uv = uvScreen;
    uv = (uv - 0.5) * 2;
    uv.x *= camera.aspectRatio;
    float z = 1.0 / tan(camera.fov * 0.5);
    vec3 rayDir = normalize(vec3(uv.x, uv.y, -z));
    rayDir = normalize(mat3(inverse(camera.view)) * rayDir);
    Ray r = { camera.position, rayDir, vec4(1, 1, 1, 1), MAX_MESH};
    return Trace(r);
}

// Multisampling raytracing
vec2[MULTISAMPLING_SIZE*MULTISAMPLING_SIZE] generateMultiSamplingUV(vec2 uv){
    vec2 step = vec2(1.0/windowSize.x, 1.0/windowSize.y) / float(MULTISAMPLING_SIZE);
    vec2[MULTISAMPLING_SIZE*MULTISAMPLING_SIZE] msUVs;
    vec2 offset = vec2(-0.5f/windowSize.x,-0.5f/windowSize.y) + vec2(0.5f/(windowSize.x*MULTISAMPLING_SIZE),0.5f/(windowSize.y*MULTISAMPLING_SIZE));
    for (uint x = 0; x < MULTISAMPLING_SIZE; x++){
        for (uint y = 0; y < MULTISAMPLING_SIZE; y++){
            msUVs[MULTISAMPLING_SIZE*x + y] = uv + offset + vec2(step.x * float(x), step.y * float(y));
        }
    }
    return msUVs;
}

vec4 multiSamplingRayTracing(){
    vec2[MULTISAMPLING_SIZE*MULTISAMPLING_SIZE] uvs = generateMultiSamplingUV(uvScreen);
    vec4 sum_color = vec4(0);
    for (uint ray_id = 0; ray_id < MULTISAMPLING_SIZE*MULTISAMPLING_SIZE; ray_id++)
    {
        vec2 uv = uvs[ray_id];
        uv = (uv - 0.5) * 2;
        uv.x *= camera.aspectRatio;
        float z = 1.0 / tan(camera.fov * 0.5);
        vec3 rayDir = normalize(vec3(uv.x, uv.y, -z));
        rayDir = normalize(mat3(inverse(camera.view)) * rayDir);
        Ray r = { camera.position, rayDir, vec4(1, 1, 1, 1), MAX_MESH};
        sum_color += Trace(r);
    }

    return sum_color/(MULTISAMPLING_SIZE*MULTISAMPLING_SIZE);
}

// Randomsampling raytracing
vec4 randomRayTracing(){
    vec4 sum_color = vec4(0);
    for (uint i = 0; i < RANDOM_SAMPLING_SIZE; i++){
        vec2 uv = uvScreen + (vec2(rand(), rand()) - 0.5) / windowSize;
        uv = (uv - 0.5) * 2;
        uv.x *= camera.aspectRatio;
        float z = 1.0 / tan(camera.fov * 0.5);
        vec3 rayDir = normalize(vec3(uv.x, uv.y, -z));
        Ray r = setWorldRay(rayDir);
        sum_color+=Trace(r);
    }
    return sum_color/RANDOM_SAMPLING_SIZE;
}

void main() {
    seed = uint(gl_FragCoord.x*100000) * 1973u + uint(gl_FragCoord.y*100000) * 9277u + uint(frame) * 26699u;

    vec4 color = basicRayTracing();

    float lambda = 1.0/float(frame);
    vec4 bufferColor = vec4(texture(readTex,uvScreen).rgb,1);
    fragColor = lambda*color + bufferColor*(1-lambda);

    // shader testing...
    //    vec4 screenPos = camera.projection * camera.view *vec4(uvScreen,0.0,1.0);
    //    fragColor = screenPos;
}