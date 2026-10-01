#version 450 core

#include "distributions.glsl"

out vec4 fragColor;
in vec2 uvScreen;

uniform sampler2D renderTex;

vec3 ACESFilm(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    vec3 texCol = texture(renderTex, uvScreen).xyz;
    texCol = xyzToSrgb(texCol);
    fragColor = vec4(ACESFilm(texCol), 1.0);
}