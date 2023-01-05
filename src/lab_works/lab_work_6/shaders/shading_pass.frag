#version 450
#define USE_BLINN_PHONG


in vec3 vLightPos; // Position de la lumière en view space ou tangent space
in vec3 vPosition; // Position du point sur la surface en view space ou tangent space

layout( binding = 0 ) uniform sampler2D uAmbientMap;
layout( binding = 1 ) uniform sampler2D uDiffuseMap;
layout( binding = 2 ) uniform sampler2D uSpecularMap;
layout( binding = 3 ) uniform sampler2D uShininessMap; 
layout( binding = 4 ) uniform sampler2D uNormalMap;

layout( location = 0 ) out vec4 fragColor;

void main() { 
	ivec2 coord = ivec2(gl_FragCoord.xy);
	// Recuperer les coordonné du pixel courant
	vec3 norm = texelFetch(uNormalMap, coord, 0).xyz;
	// Recuperer les données depuis les texture
	vec3 amb = texelFetch(uAmbientMap, coord, 0).xyz;
	vec3 diff = texelFetch(uDiffuseMap, coord, 0).xyz;
	vec3 spec = texelFetch(uSpecularMap, coord, 0).xyz;
	vec4 shin = texelFetch(uShininessMap, coord, 0); 
	

	// Camera en 0, 0 donc direction de la camera par rapport à l'objet = 0 - posistion du point 
	vec3 dirCamera = normalize(-vPosition); // Direction de la camera en tangente space = Lout
	vec3 dirLum = normalize(vLightPos - vPosition); // -Li 
		 
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

	fragColor = vec4(amb + 
		diff * max(cosAngleDiff, 0) + 
		shin.xyz * pow( 
			max(cosAngleSpec, 0), 
			shin.w
		),
	1) ;
}
