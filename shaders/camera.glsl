#ifndef CAMERA_GLSL
#define CAMERA_GLSL

struct Camera {
    vec3 position;
    mat4 view;
    mat4 projection;
    vec3 up;
    float fov;
    float aspectRatio;
    float nearPlane;
    float farPlane;
    float apertureRadius;
    float focusDistance;
};

#endif