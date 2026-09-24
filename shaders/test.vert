#version 330 core

layout(location = 0) in vec2 pos;

out vec4 fragPos;

void main() {
    fragPos = vec4(pos, 0, 1);
    gl_Position = vec4(pos,0.0, 1.0);
}
