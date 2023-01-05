#version 450
#define USE_BLINN_PHONG
//#define LIGHT_FOLLOW_CAMERA

uniform vec3 uDiffuse;
uniform vec3 uSpecular;
uniform float uShininess;  
uniform vec3 uLumPos;

in vec3 vColor;
in vec3 vNormal;
in vec3 vViewSpacePosition;

layout( location = 0 ) out vec4 fragColor;

void main() { 
	// Camera en 0, 0 donc direction de la camera par rapport à l'objet = 0 - posistion du point
	vec3 dirCamera = normalize(-vViewSpacePosition); // Direction de la camera en view space = Lout
	#ifdef LIGHT_FOLLOW_CAMERA
		vec3 dirLum = dirCamera; // Direction de la lumière par rapport à l'objet en view space = - Lin
	#else
		vec3 dirLum = normalize(uLumPos - vViewSpacePosition); 
	#endif

	vec3 norm = normalize(vNormal);	
	if (dot(norm, dirCamera) < 0) // Flip les normales si elle ne sont pas tourné vers la camera
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

	fragColor = vec4(vColor + 
		uDiffuse * max(cosAngleDiff, 0) + 
		uSpecular * pow( 
			max(cosAngleSpec, 0), 
			uShininess
		),
	1) ;
}