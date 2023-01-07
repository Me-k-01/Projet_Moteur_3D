#version 450

in vec3 vNormal; 
in vec2 vTextCoords; 

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

layout( location = 0 ) out vec3 gFragPosition;
layout( location = 1 ) out vec3 gNormals;
layout( location = 2 ) out vec3 gAmbiant;
layout( location = 3 ) out vec3 gDiffuse;
layout( location = 4 ) out vec4 gSpecular;

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

	 
	/*vec3 norm; 
	if (uHasNormalMap) { // Avec normal map: calcul en Tangente space 
		norm = texture(uNormalMap, vTextCoords).rgb;
		norm = normalize(norm * 2.0 - 1.0);
	} else { // Sans normal map: calcul en View Space
		norm = normalize(vNormal); 
	} */
	vec3 norm = normalize(vNormal); 


	gFragPosition = gl_FragCoord.xyz;
	gAmbiant = coefAmbient;
	gDiffuse = coefDiffuse;
	gSpecular = vec4(coefSpecular.xyz, coefShininess);
	gNormals = norm;
}