#version 450
//#define LIGHT_FOLLOW_CAMERA

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
uniform mat4 uMVMatrix; 
uniform mat4 uMVPMatrix; // Projection * View * Model 
uniform mat4 uNormalMatrix; 

// --- Sortie
out vec2 vTextCoords; 
out vec3 vLightPos; 
out vec3 vPosition;
out vec3 vNormal;

void main()
{
	vec4 pos = vec4( aVertexPosition, 1 );
	gl_Position = uMVPMatrix * pos; // Position dans le model space 
	vTextCoords = aVertexTexCoords;
	   
	vNormal = normalize(mat3(uNormalMatrix) * aVertexNormal); // N 
	vec3 position = vec3(uMVMatrix * pos);

	if (uHasNormalMap) {
		vec3 tang = normalize(mat3(uNormalMatrix) * aVertexTangent); // T
		vec3 bitang = normalize(mat3(uNormalMatrix) * aVertexBitagent); // B
		// Re-orthonormer
		tang = normalize ( tang - dot (tang , vNormal ) * vNormal ); // On veut T orthogonal a N
		bitang = cross (vNormal , tang );


		mat3 matrixTBN = mat3(tang.xyz, bitang.xyz, vNormal.xyz);
		mat3 inv_TBN = transpose(matrixTBN);
	
		// Position dans le tangent space du point sur la surface 
		vPosition = inv_TBN * position; // Position dans le tangent space
		vLightPos = inv_TBN * uLumPos; 
	} else {
		// Position dans le view space du point sur la surface 
		vPosition = position;	
		vLightPos = uLumPos; 
	}
}
