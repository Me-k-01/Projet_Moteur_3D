#version 450
#define USE_BLINN_PHONG
#define NUM_LIGHTS 3

in vec3 vNormal;
in vec2 vTextCoords; 
in vec3 vPosition; // Position du point sur la surface en view space ou tangent space 
in vec3 vLightsPos[NUM_LIGHTS];
uniform vec3 uLightsTint[NUM_LIGHTS]; 

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

uniform int uAmbientType; // 1: normal, 2 et 3: texture procéduraux 
uniform vec2 uResolution; 
uniform float uTime;
uniform float uStrength;
 
  

  //////////////////////////// Bruit ////////////////////////////

// Source de l'algorithme: https://thebookofshaders.com/12/
#ifdef GL_ES
precision mediump float;
#endif

vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

vec3 cellularNoise(vec2 fragCoord) { 
    vec2 st = fragCoord/uResolution.xy;
    st.x *= uResolution.x/uResolution.y;
    vec3 color = vec3(.0);

    // Scale
    st *= 3.;

    // Tile the space
    vec2 i_st = floor(st);
    vec2 f_st = fract(st);

    float m_dist = 1.;  // minimum distance

    for (int y= -1; y <= 1; y++) {
        for (int x= -1; x <= 1; x++) {
            // Neighbor place in the grid
            vec2 neighbor = vec2(float(x),float(y));

            // Random position from current + neighbor place in the grid
            vec2 point = random2(i_st + neighbor);

			// Animate the point
            point = 0.5 + 0.5*sin(uTime + 6.2831*point);

			// Vector between the pixel and the point
            vec2 diff = neighbor + point - f_st;

            // Distance to the point
            float dist = length(diff);

            // Keep the closer distance
            m_dist = min(m_dist, dist);
        }
    }

    // Draw the min distance (distance field)
    color += m_dist;

    // Draw cell center
    //color += 1.-step(.02, m_dist);

    // Draw grid red
    //color.r += step(.98, f_st.x) + step(.98, f_st.y);

    // Show isolines
    if (uAmbientType == 2)
        color -= step(.7,abs(sin(27.0*m_dist)))*.5;

    return color;
}




// copy from https://www.shadertoy.com/view/4l2GzW
float r(float n)
{
 	return fract(cos(n*89.42)*343.42);
}
vec2 r(vec2 n)
{
 	return vec2(r(n.x*23.62-300.0+n.y*34.35),r(n.x*45.13+256.0+n.y*38.89)); 
}
float worley(vec2 n,float s)
{
    float dis = 2.0;
    for(int x = -1;x<=1;x++)
    {
        for(int y = -1;y<=1;y++)
        {
            vec2 p = floor(n/s)+vec2(x,y);
            float d = length(r(p)+vec2(x,y)-fract(n/s));
            if (dis>d)
            {
             	dis = d;   
            }
        }
    }
    return 1.0 - dis;
}
// copy from https://www.shadertoy.com/view/4sc3z2
#define MOD3 vec3(.1031,.11369,.13787)
vec3 hash33(vec3 p3)
{
	p3 = fract(p3 * MOD3);
    p3 += dot(p3, p3.yxz+19.19);
    return -1.0 + 2.0 * fract(vec3((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y, (p3.y+p3.z)*p3.x));
}
float perlin_noise(vec3 p) {
    vec3 pi = floor(p);
    vec3 pf = p - pi;
    
    vec3 w = pf * pf * (3.0 - 2.0 * pf);
    
    return 	mix(
        		mix(
                	mix(dot(pf - vec3(0, 0, 0), hash33(pi + vec3(0, 0, 0))), 
                        dot(pf - vec3(1, 0, 0), hash33(pi + vec3(1, 0, 0))),
                       	w.x),
                	mix(dot(pf - vec3(0, 0, 1), hash33(pi + vec3(0, 0, 1))), 
                        dot(pf - vec3(1, 0, 1), hash33(pi + vec3(1, 0, 1))),
                       	w.x),
                	w.z),
        		mix(
                    mix(dot(pf - vec3(0, 1, 0), hash33(pi + vec3(0, 1, 0))), 
                        dot(pf - vec3(1, 1, 0), hash33(pi + vec3(1, 1, 0))),
                       	w.x),
                   	mix(dot(pf - vec3(0, 1, 1), hash33(pi + vec3(0, 1, 1))), 
                        dot(pf - vec3(1, 1, 1), hash33(pi + vec3(1, 1, 1))),
                       	w.x),
                	w.z),
    			w.y);
}
// Perlin + Cellular : https://www.shadertoy.com/view/MdGSzt
vec3 perlinCellular( vec2 fragCoord ) {
    float dis = (1.0+perlin_noise(vec3(fragCoord.xy/uResolution.xy, uTime*0.05)*8.0)) 
        * (1.0+(worley(fragCoord.xy, 32.0)+
        0.5*worley(2.0*fragCoord.xy,32.0) +
        0.25*worley(4.0*fragCoord.xy,32.0) ));
	return vec3(dis/4.0);
}
//////////////////////////////////////////////////////////////////////////////////////////////////

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
	
	vec3 coefAmbient;
    if (uAmbientType == 0) { 	  
		coefAmbient = uHasAmbientMap ? texture(uAmbientMap, vTextCoords).xyz : uAmbient;
    } else if (uAmbientType == 3) { // On affiche les textures procédurales
        // perlin_noise 
        coefAmbient = vec3(perlin_noise(vec3(vTextCoords, 1))) * uStrength;
    } else if (uAmbientType == 4) {
        coefAmbient = vec3(perlinCellular(vTextCoords)) * uStrength;
    } else {
        coefAmbient = cellularNoise(vTextCoords) * uStrength;
	}
	vec3 coefSpecular = uHasSpecularMap ? texture(uSpecularMap, vTextCoords).xxx : uSpecular;
	float coefShininess = uHasShininessMap ? texture(uShininessMap, vTextCoords).x : uShininess; 

	vec3 norm;

	// Camera en 0, 0 donc direction de la camera par rapport à l'objet = 0 - posistion du point 
	vec3 dirCamera = normalize(-vPosition); // Direction de la camera en tangente space = Lout
		
	if (uHasNormalMap) { // Avec normal map: calcul en Tangente space 
		norm = texture(uNormalMap, vTextCoords).rgb;
		norm = normalize(norm * 2.0 - 1.0);
	} else { // Sans normal map: calcul en View Space
		norm = normalize(vNormal); 
	}
	// Flip les normales si elle ne sont pas tourné vers la camera
	if (dot(norm, dirCamera) < 0) 
		norm = - norm;
		

	// Pour chaque lumière
	vec3 col = vec3(0);
	for (int i = 0; i < NUM_LIGHTS; i++) {
	
		vec3 dirLum = normalize(vLightsPos[i] - vPosition); // -Li 
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

		col += (coefAmbient + 
			coefDiffuse * max(cosAngleDiff, 0) + 
			coefSpecular * pow( 
				max(cosAngleSpec, 0), 
				coefShininess
			)
		) * uLightsTint[i]; 
		
	}
	fragColor = vec4(col, 1);
}