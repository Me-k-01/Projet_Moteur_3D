#version 450
//#define LIGHT_FOLLOW_CAMERA

layout( location = 0 ) in vec3 aVertexPosition;
layout( location = 1 ) in vec3 aVertexNormal;
layout( location = 2 ) in vec2 aVertexTexCoords;
layout( location = 3 ) in vec3 aVertexTangent;
layout( location = 4 ) in vec3 aVertexBitagent;


uniform mat4 uMVMatrix; 
uniform mat4 uMVPMatrix; // Projection * View * Model 
uniform mat4 uNormalMatrix;
uniform vec3 uAmbiant; 
//uniform vec3 uLumPos;

out vec3 vColor;
out vec3 vNormal;
//out vec3 vLumDir; // Direction de la lumière dans le view space
out vec3 vViewSpacePosition;

void main()
{
	vec4 pos = vec4( aVertexPosition, 1 );
	gl_Position = uMVPMatrix * pos; // Position dans le model space 

    vColor = uAmbiant;
	vNormal = normalize(mat3(uNormalMatrix) * aVertexNormal); 
	// Position dans le view space du point sur la surface
	vViewSpacePosition = vec3(uMVMatrix * pos);
}
