#version 450
#define USE_BLINN_PHONG


// --- Entrée

// --- Uniforme
uniform vec3 uLumPos; // En view space 
uniform mat4 uMVMatrix; 

uniform sampler2D gAmbientMap;
uniform sampler2D gDiffuseMap;
uniform sampler2D gSpecularMap;
uniform sampler2D gShininessMap; 
uniform sampler2D gNormalMap;

out vec4 fragColor;

void main() { 
	ivec2 coord = ivec2(gl_FragCoord.xy);
	// Recuperer les coordonné du pixel courant
	vec3 norm = texelFetch(gNormalMap, coord, 0).xyz;
	// Recuperer les données depuis les texture
	vec3 amb = texelFetch(gAmbientMap, coord, 0).xyz;
	vec3 diff = texelFetch(gDiffuseMap, coord, 0).xyz;
	vec3 spec = texelFetch(gSpecularMap, coord, 0).xyz;
	vec4 shin = texelFetch(gShininessMap, coord, 0); 
	 
	// Position dans le view space du point sur la surface 
	vec3 pos = vec3(uMVMatrix * gl_FragCoord.xyzw);	
	vec3 lightPos = uLumPos; 
	// Camera en 0, 0 donc direction de la camera par rapport à l'objet = 0 - posistion du point 
	vec3 dirCamera = normalize(-pos); // Direction de la camera en tangente space = Lout
	vec3 dirLum = normalize(lightPos - pos); // -Li 
		 
	// Flip les normales si elle ne sont pas tourné vers la camera
	if (dot(norm, dirCamera) < 0) 
		norm = - norm;
	 
	 // Diffuse
	float cosAngleDiff = dot(norm, dirLum) ; 
	// Speculaire 
	#ifdef USE_BLINN_PHONG
		vec3 h = normalize(dirCamera + dirLum); // demi vecteur entre Lo et Li, norm(Lo - Li) 
		float cosAngleSpec = dot(norm, h);
	#else
		vec3 r = reflect(-dirLum, norm); 
		float cosAngleSpec = dot(dirCamera, r); 
	#endif
	 
	 fragColor = vec4(amb, 1);
	/*
	fragColor = vec4(amb + 
		diff * max(cosAngleDiff, 0) + 
		shin.xyz * pow( 
			max(cosAngleSpec, 0), 
			shin.w
		),
	1) ;*/
	
}
