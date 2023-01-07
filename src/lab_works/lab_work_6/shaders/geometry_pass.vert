#version 450

// --- Entrée
layout( location = 0 ) in vec3 aVertexPosition;
layout( location = 1 ) in vec3 aVertexNormal;
layout( location = 2 ) in vec2 aVertexTexCoords;
layout( location = 3 ) in vec3 aVertexTangent;
layout( location = 4 ) in vec3 aVertexBitagent;

// --- Uniforme
uniform vec3 uLumPos;
uniform bool uHasNormalMap;
// Matrix 
uniform mat4 uMVPMatrix; // Projection * View * Model 
uniform mat4 uNormalMatrix; 

// --- Sortie
out vec2 vTextCoords; 
out vec3 vNormal;

void main()
{
	vec4 pos = vec4( aVertexPosition, 1 );
	gl_Position = uMVPMatrix * pos; // Position dans le model space 
	vTextCoords = aVertexTexCoords;
	   
	vNormal = normalize(mat3(uNormalMatrix) * aVertexNormal); // N 
}
