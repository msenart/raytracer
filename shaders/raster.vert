#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 tangent;
layout(location = 3) in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 worldPos;
out vec3 worldNormal;

void main()
{
    vec4 wp = model * vec4(position, 1.0);
    worldPos = wp.xyz;
    worldNormal = mat3(transpose(inverse(model))) * normal;
    gl_Position = projection * view * wp;
}
