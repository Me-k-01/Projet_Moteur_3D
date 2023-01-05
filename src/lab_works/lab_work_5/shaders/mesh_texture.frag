#version 450
#define USE_BLINN_PHONG

in vec3 vNormal;
/*
in vec3 vTangSpaceLumPos;
in vec3 vViewSpaceLumPos;
in vec3 vTangSpacePosition;
in vec3 vViewSpacePosition;*/
in vec2 vTextCoords;
in vec3 vLightPos; // Position de la lumière en view space ou tangent space
in vec3 vPosition; // Position du point sur la surface en view space ou tangent space

uniform vec3 uAmbient;  
uniform vec3 uDiffuse;
uniform vec3 uSpecular;
uniform float uShininess;  

uniform bool uHasAmbientMap;
uniform bool uHasDiffuseMap;
uniform bool uHasSpecularMap;
uniform bool uHasShininessMap;
uniform bool uHasNormalMap;

layout( binding = 0 ) uniform sampler2D uAmbientMap;
layout( binding = 1 ) uniform sampler2D uDiffuseMap;
layout( binding = 2 ) uniform sampler2D uSpecularMap;
layout( binding = 3 ) uniform sampler2D uShininessMap; 
layout( binding = 4 ) uniform sampler2D uNormalMap;

layout( location = 0 ) out vec4 fragColor;

void main() { 
	vec3 coefDiffuse = uDiffuse;
	float alpha = 1;
	if (uHasDiffuseMap) {
		vec4 textDiff = texture(uDiffuseMap, vTextCoords);
		coefDiffuse = textDiff.xyz;
		alpha = textDiff.w;
		if (alpha < 0.5) 
			discard;
	}
	vec3 coefAmbient = uHasAmbientMap ? texture(uAmbientMap, vTextCoords).xyz : uAmbient;
	vec3 coefSpecular = uHasSpecularMap ? texture(uSpecularMap, vTextCoords).xxx : uSpecular;
	float coefShininess = uHasShininessMap ? texture(uShininessMap, vTextCoords).x : uShininess;

	vec3 norm;

	// Camera en 0, 0 donc direction de la camera par rapport à l'objet = 0 - posistion du point 
	vec3 dirCamera = normalize(-vPosition); // Direction de la camera en tangente space = Lout
	vec3 dirLum = normalize(vLightPos - vPosition); // -Li 
		
	if (uHasNormalMap) { // Avec normal map: calcul en Tangente space 
		norm = texture(uNormalMap, vTextCoords).rgb;
		norm = normalize(norm * 2.0 - 1.0);
	} else { // Sans normal map: calcul en View Space
		norm = normalize(vNormal); 
	}
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

	fragColor = vec4(coefAmbient + 
		coefDiffuse * max(cosAngleDiff, 0) + 
		coefSpecular * pow( 
			max(cosAngleSpec, 0), 
			coefShininess
		),
	1) ;
}