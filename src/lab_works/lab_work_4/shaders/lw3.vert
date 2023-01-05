#version 450

layout ( location = 0 ) in vec3 aVertexPosition;
layout ( location = 1 ) in vec3 aVertexColor;
uniform mat4 uMVPMatrix;
uniform vec3 uAmbiant;
uniform vec3 uDiffuse;

out vec4 vColor;

void main() {
    gl_Position = uMVPMatrix * vec4(aVertexPosition, 1.f);
    vColor = vec4(uAmbiant, 1.f);
}
