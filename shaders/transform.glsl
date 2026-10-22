#version 120

layout(std430, binding = 5) readonly buffer TransformsBuffer {
    mat4 transforms[];
};