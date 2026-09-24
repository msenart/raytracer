#version 330 core

layout(location = 0) in vec2 pos;

out vec4 fragColor;
in vec4 fragPos;

uniform mat4 view;

void main() {
    vec4 screenPos = view * fragPos;

    fragColor = screenPos;
}
