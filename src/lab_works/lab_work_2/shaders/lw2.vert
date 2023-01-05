#version 450

layout ( location = 0 ) in vec2 aVertexPosition;
layout ( location = 1 ) in vec4 aVertexColor;
uniform float uTranslationX;


out vec4 vColor;

void main() {
    gl_Position = vec4(aVertexPosition[0] + uTranslationX, aVertexPosition[1], 0.f, 1.f);
    vColor = aVertexColor;
}
