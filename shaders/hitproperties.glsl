#ifndef HITPROPERTIES_GLSL
#define HITPROPERTIES_GLSL

struct HitProperties{
    bool hasHit;
    uint argmin_mesh_type;
    uint argmin_mesh;
    float t;
    vec2 uv;
    vec3 normal;
};

#endif
