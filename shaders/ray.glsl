#ifndef RAY_GLSL
#define RAY_GLSL

struct Ray {
    vec3 o;
    vec3 d;
    float throughput;
    float lambda;
    uint in_objtype_id;
    uint in_obj_id;
};

#endif
