#version 450

in vec4 vColor;
layout ( location = 0 ) out vec4 fragColor;
uniform float uLum;

void main() {
    fragColor = vec4(vColor[0] * uLum, vColor[1] * uLum, vColor[2] * uLum, vColor[3]);
}
