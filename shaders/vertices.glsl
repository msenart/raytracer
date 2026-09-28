#ifndef VERTICES_GLSL
#define VERTICES_GLSL

struct Vertex{
    vec3 position;  float _pad0;
    vec3 normal;    float _pad1;
    vec4 tangent;
    vec2 uv;        vec2 _pad2;
};

struct IndexAndMeshID{
    uint index;
    uint mesh_id;
    uint _pad0;
    uint _pad1;
};

// Vertices SSBO

layout(std430, binding = 0) readonly buffer VertexBuffer {
    Vertex vertices[];
};

// Indices SSBO

layout(std430, binding = 1) readonly buffer IndicesBuffer {
    IndexAndMeshID indices[];
};

#endif