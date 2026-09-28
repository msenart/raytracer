#ifndef HITPROPERTIES_GLSL
#define HITPROPERTIES_GLSL

struct HitProperties{
    bool hasHit;
    float t;
    vec2 uv;
    vec3 normal;
};

#endif
