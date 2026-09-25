#version 450 core

out vec4 fragColor;
in vec2 uvScreen;

uniform sampler2D renderTex;

void main()
{
    vec3 texCol = texture(renderTex, uvScreen).rgb;
//    fragColor = sqrt(vec4(texCol, 1.0));
    fragColor = vec4(texCol, 1.0);
}