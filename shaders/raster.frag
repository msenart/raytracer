#version 450 core

in vec3 worldPos;
in vec3 worldNormal;

out vec4 fragColor;

uniform vec3 color;
uniform vec3 cameraPos;
uniform bool emissive;
uniform bool selected;

const vec3 SELECTION_COLOR = vec3(1.0, 0.55, 0.05);

void main()
{
    vec3 V = normalize(cameraPos - worldPos);
    vec3 N = normalize(worldNormal);
    N = faceforward(N, -V, N); // two sided shading, so we don't depend on the winding order

    vec3 lit = color;
    if (!emissive) {
        float headlight = max(dot(N, V), 0.0);
        float hemisphere = 0.5 + 0.5 * N.y;
        lit = color * (0.15 + 0.35 * hemisphere + 0.5 * headlight);
    }

    if (selected) {
        float rim = pow(1.0 - max(dot(N, V), 0.0), 2.0);
        lit = mix(lit, SELECTION_COLOR, 0.25 + 0.6 * rim);
    }
    fragColor = vec4(lit, 1.0);
}
