
#version 400


#define MAX_LIGHTS_NUMBER 100
#define PI 3.14159265359f

// If 1 the depth buffer is wrote with a linear range between near and far plane
#define LINEAR_DEPTH 0

// 0 from model info 1 from Normal map 
#define NORMAL_MAP 0

// 0 no shadows, 1 simple LEQUAL test, 2 for PCF, 3 for VSM
#define SHADOW 1


// 0 constant values for rho_s and roughness given via uniforms, 1 defined via texture
#define SPECULAR 0

/*  Distribution term (D)											*
*	  	- Blinn [0]													*
*	  	- GGX  (trowbridge reitz) [1]	         					*
*	  	- Beckmanm [2] (default)									*

*	  Fresnel Term (F)												*
*		- None [0] (just F0)										*
*		- Cook-Torrance [1] 										*
*		- Schlick [2] (default)										*
*																	*
*	  Geometric Term (G)											*
*	  	- Implicit [0]												*
*	  	- Neumann [1]												*
*	  	- Cook-Torrance [2]											*
*	  	- Kelemen-Kalos [3] (default)								*
*	  	- Smith - Walter [4]										*
*	  	- Smith - Schlick/Beckmann [5]								*
*/
#define DISTRIBUTION 2
#define FRESNEL 2
#define GEOMETRIC 3


// 0 no usage of melanine info 1 constant values across face 2 values defined via texture
#define MELANINE 0 

// 0 no usage of hemoglobine info 1 constant values across face 2 values defined via texture
#define HEMOGLOBINE 0

#define SPEC_ONLY 0

// 0 specular is rendered 1 is not
#define NO_SPEC 0

// If 1 a pow(2.2) is applied to the diffuse texture
#define GAMMA 0

// If 1 only the colour texture is shown
#define JUST_COLOR 0


//// DEBUG OPTION.
// If 1 no diffuse colour is shown
#define DEBUG_NO_COL 0
// If 1 the normal is shown instead of proper fragment colour		
#define DEBUG_SHOW_NORMAL 0



in vec2 vTextureCoordinate;
in vec4 worldSpacePosition;
in vec3 V;

#if NORMAL_MAP == 1
in vec3 t;
in vec3 n;
in vec3 b;
#elif NORMAL_MAP == 0
in vec3 vVertexNormal;
#endif


// All the lights are loaded once in a UBO. As the shader render considering 
// just one light, the index of the light used for the current rendering must be 
// passed as uniform
uniform int lightIndex;

// This struct respect the std140 layout
struct Light{
	vec4 position;
	vec4 intensity;
	vec4 attenuationTerms;
};

layout(std140) uniform Lights {
	Light lights[MAX_LIGHTS_NUMBER];
};

uniform vec3 ambientColour;
uniform sampler2D diffuseTexture;

#if NORMAL_MAP == 1
uniform sampler2D normalTexture;
#endif 


#if MELANINE == 1
uniform float melConcentration;
#elif MELANINE == 2
uniform sampler2D melanineTexture;
#endif

#if HEMOGLOBINE == 1
uniform float hemConcentration;
#elif HEMOGLOBINE == 2
uniform sampler2D hemoglobineTexture;
#endif

#if (MELANINE != 0 && HEMOGLOBINE != 0)
uniform sampler2D skinColourTexture;
#endif

// In case of fixed specular parameters


#if SPECULAR == 1
uniform sampler2D specParamTex; 
#else // Fixed parameters
uniform float roughness;
uniform float rho_s;
#endif

uniform float F0;


#if SHADOW != 0
uniform sampler2D shadowTexture;
uniform vec2 shadowPixelSize;	// 1/shadowMap resolution
in vec4 shadowCoordinates;
#endif

// Output colour
out vec4 fragmentColor;


#if LINEAR_DEPTH == 1
// If the depth must be rendered linearly, the perspective matrix is used to 
// extract the clip-planes informations. 
uniform mat4 pMatrix;
#endif

/******************* Utils function ***********************/
#if LINEAR_DEPTH == 1

float linearValue(float value, float near, float far){
	value = value * 2.0 - 1.0;
	return 2.0 * near * far / (far + near - value * (far - near));
}

float linearDepth(float depth){
	float n = (pMatrix[3][2]) / (pMatrix[2][2] - 1.0);
	float f = (pMatrix[3][2]) / (pMatrix[2][2] + 1.0);
	return linearValue(depth, n, f);
}
#endif


/********************************************************
* Specular Torrance Sparrow BRDF terms									*
*********************************************************/

/******* Distribution terms *******/

// Not common form but follows when power = 2/m^2 - 2 
// See http://graphicrants.blogspot.co.uk/ taken as reference. 
float blinnD(float roughness, float NdotH){
	float roughness2 = roughness*roughness;
	float roughness2Inverse = 1.0/roughness2;
	float term1 = pow(NdotH,roughness2Inverse*2.0 -2.0);
	return term1*roughness2Inverse;
}

float GGXD(float roughness,float NdotH){
	
	float roughness2 = roughness*roughness;
	float denominator = ((NdotH*NdotH*(roughness2-1.0))+1.0);
	return roughness2/(denominator*denominator);
} 

float beckmannD(float roughness,float NdotH){
	if(NdotH==0.0)
		return 0.0;	
	float roughness2 = roughness*roughness;
	float NdotH2 = NdotH*NdotH;
	float term1 = 1.0/(roughness2*NdotH2*NdotH2);
	float expTerm = exp((NdotH2 - 1.0)/(roughness2*NdotH2));
	return term1*expTerm;
} 


/******* Geometric terms *******/
// Note: NdotV and NdotL are removed as they would simplify anyway. Special case for Cook-Torrance G that 
// is therefore different from what could be expected.

float implicitG(float NdotL, float NdotV){
	// return NdotL*NdotV;
	return 	1.0;
}

float neumannG(float NdotL, float NdotV){
	//float numerator = NdotL*NdotV;
	float numerator = 1;
	float denominator = max(NdotL,NdotV);
	return (numerator/denominator);
}


float cookTorranceG(float NdotL, float NdotV, float NdotH, float VdotH){
	if((NdotL * NdotV)==0 || (VdotH)==0)
		return 0.0;

	float term1 = (2.0*NdotH*NdotV) / VdotH;
	float term2 = (2.0*NdotH*NdotL) / VdotH;
	float properReturn =  min(1.0,min(term1,term2)); // What the theoretical term should return.

	return (properReturn/(NdotL*NdotV)); // To compensate the removal of (NdotL*NdotV) from the denominator of the full BRDF
}

float kelemenG(float NdotL, float NdotV, float LdotV){
	//float numerator = 2*NdotL*NdotV;
	float numerator = 2;
	float denominator = 1+LdotV;
	return numerator/denominator;
}

// Double check!
float WalterG(float NdotL, float NdotV, float HdotV, float HdotL, float roughness){

	if(NdotL==0 || NdotV ==0)
		return 0;

	float a = 1.0 / (roughness * tan(acos(NdotV)));
	float term;
	if(a<1.6){
		float a2= a*a;
		float numerator = 3.535*a + 2.181*a2;
		float denominator = 1 + 2.276*a + 2.577*a2;
		term = numerator/denominator; 
	}else term = 1;

	float properReturn = (step(0.0,HdotL/NdotL)*term) * (step(0.0,HdotV/NdotV)*term);

	return (properReturn/(NdotL*NdotV)); // To compensate the removal of (NdotL*NdotV) from the denominator of the full BRDF
}

float SchlickG(float NdotL, float NdotV, float roughness){
	// float numerator = NdotV * NdotL;  // Removed to avoid division at the end.
	float k = roughness * sqrt(2/PI);
	float G1_v = 1/(NdotV*(1-k)+k);
	float G1_l = 1/(NdotL*(1-k)+k);

	return G1_v*G1_l;
}


/******* Fresnel terms *******/

// Schlick Fresnel Approx. 
// From GPU Gems 3 
float schlickF(float VdotH){
	float base = 1.0 - VdotH;
	float exponential = pow(base,5.0);
	return exponential + F0 * (1.0 - exponential);
}

float cookTorranceF(float VdotH){

	float sqrtF0 = sqrt(F0);
	float eta = (1+sqrtF0) / (1-sqrtF0);
	float g = sqrt(eta*eta + VdotH*VdotH -1);
	float term1 = 0.5*((g-VdotH)/(g+VdotH))*((g-VdotH)/(g+VdotH));
	float term2 = ((g+VdotH)*VdotH - 1)/((g-VdotH)*VdotH + 1);
	term2 = 1 + term2*term2;

	return term1*term2;
}




/***************************************************************************************************************************************************/

/*
Compute the specular BRDF factor according to the D,F and G choices
*/
float computeBRDF(float roughness,vec3 L, vec3 N, vec3 V){
	vec3 H = normalize(V+L);
	float NdotL = max(dot(N,L),0.0);
	float NdotV = max(dot(N,V),0.0);
	float NdotH = max(dot(N,H),0.0);	

	float D=0;
	float F=0;
	float G=0;


	#if DISTRIBUTION == 0
		D = blinnD(roughness,NdotH);
	#elif DISTRIBUTION == 1 
		D = GGXD(roughness,NdotH);
	#elif DISTRIBUTION == 2
		D = beckmannD(roughness, NdotH);	
	#endif


	#if GEOMETRIC == 0
		G = implicitG(NdotL,NdotV); 
	#elif GEOMETRIC == 1
		G = neumannG(NdotL, NdotV);
	#elif GEOMETRIC == 2
		float VdotH = max(dot(V,H),0.0);	
		G = cookTorranceG(NdotL, NdotV, NdotH, VdotH);
	#elif GEOMETRIC == 3
		float LdotV = max(dot(L,V),0.0);
	 	G = kelemenG(NdotL, NdotV, LdotV);
	#elif GEOMETRIC == 4
		float VdotH = max(dot(V,H),0.0);	
		float HdotL = max(dot(H,L),0.0);
		G = WalterG(NdotL, NdotV, VdotH, HdotL, roughness);
	#elif GEOMETRIC == 5
		G = SchlickG(NdotL, NdotV, roughness);	
	#endif		


	#if FRESNEL == 0 
		F = noneF(F0);
	#endif
	#if (FRESNEL == 1 && (GEOMETRIC == 2 || GEOMETRIC == 4))
		F = cookTorranceF(F0,VdotH);
	#endif
	#if (FRESNEL == 1 && !(GEOMETRIC == 2 || GEOMETRIC == 4)) // VdotH not defined before	
		float VdotH = max(dot(V,H),0.0); 			
		F = cookTorranceF(F0,VdotH);
	#endif
	#if (FRESNEL == 2 && (GEOMETRIC == 2 || GEOMETRIC == 4))
		F = schlickF(F0, VdotH);
	#endif
	#if (FRESNEL == 2 && !(GEOMETRIC == 2 || GEOMETRIC == 4)) // VdotH not defined before	
		float VdotH = max(dot(V,H),0.0);			
		F = schlickF(VdotH);
	#endif

	return (D*F*G) / (4.0 * PI);
}

/*________________________________________________________ */


float computeAttenuation(Light light, float d){
	return 1 / (light.attenuationTerms.x + (d*light.attenuationTerms.y) + (d*d*light.attenuationTerms.z));
}


#if NORMAL_MAP == 1
vec3 normalFromMap(){
	// Normalize the tangent basis
	vec3 T = normalize(t); vec3 N = normalize(n); vec3 B = normalize(b);
	mat3 TBN = (mat3(T,B,N));
	// Get and return normal in world space
	vec3 normal = normalize(texture(normalTexture, vTextureCoordinate).rgb*2.0 - 1.0);
	return normalize(TBN*normal);
}
#endif


#if (HEMOGLOBINE != 0 && MELANINE != 0)
vec4 skinColour(){

	#if HEMOGLOBINE == 1
		float Ch = hemConcentration;
	#elif HEMOGLOBINE == 2
		float Ch = texture(hemoglobineTexture,vTextureCoordinate).r;
	#endif

	#if MELANINE == 1
		float Cm = melConcentration;
	#elif MELANINE == 2
		float Cm = texture(melanineTexture,vTextureCoordinate).r;
	#endif

	// Obtain skin colour from the skin colour LUT using values of 
	//melanin and haemoglobin concentration
	Ch = Ch/0.32;
	Cm = Cm / 0.5;
	float u = pow(Cm, (1.0/3.0));
	float v = 1.0-pow(Ch , (1.0/3.0));
	vec4 tone=texture(skinColourTexture, vec2(u,v));


	return tone;
}
#endif

#if SHADOW == 3

float shadowVSM(vec3 screenSpaceCoords, float testPointZ){

	vec2 moments = texture(shadowTexture, screenSpaceCoords.xy).rg;

	if(testPointZ <= moments.x) return 1.0;

	// Check how likely a pixel is lit. )
	float variance = moments.y - (moments.x*moments.x);
	variance = max(variance, 0.0002); //bound the variance
	float d = (testPointZ) - moments.x;
	float pMax = variance / (variance + d*d);

	return pMax;

}


#elif SHADOW == 2

// PCF. samples MUST be odd
float shadowPCF(vec3 screenSpaceCoords, float testPointZ, float samples){	

	float biasEpsilon = -0.005; 
	// PCF specific code
	float offset = (samples - 1.0)/2.0;
	float visibilityFactor = 0.0;

	for(float x = -offset; x<=offset; x+=1.0){
		for(float y = -offset; y<=offset; y+=1.0){

			vec2 fetchOffset = vec2(x,y)*shadowPixelSize;
			float occluderZ = texture(shadowTexture, screenSpaceCoords.xy + fetchOffset).r;

			if(occluderZ > testPointZ + biasEpsilon)
				visibilityFactor += 1.0; 
		}
	}

	return (visibilityFactor/(samples*samples));
}

#elif SHADOW == 1

float shadowBasicTest(vec3 screenSpaceCoords, float testPointZ){
	float biasEpsilon =  -0.005;  
	float occluderZ = texture(shadowTexture, screenSpaceCoords.xy).r;
	if(occluderZ < testPointZ + biasEpsilon)
		return 0.3; // in shadow  
	else
		return 1.0; 

}

#endif

#if SHADOW != 0
// Determine the visibility of the pixel
float visibility(){
	
	vec3 projCoordinates = shadowCoordinates.xyz / shadowCoordinates.w;
	vec3 screenSpaceCoords = 0.5 + projCoordinates*0.5;
	float testPointZ = screenSpaceCoords.z;

	#if SHADOW == 1
	return shadowBasicTest(screenSpaceCoords, testPointZ);
	#elif SHADOW == 2
	return shadowPCF(screenSpaceCoords, testPointZ, 3.0);
	#elif SHADOW == 3
	return shadowVSM(screenSpaceCoords, testPointZ);
	#endif
}
#endif




void main(void){

	// Get normal
	#if NORMAL_MAP == 0
	 vec3 N = normalize(vVertexNormal);
	#elif NORMAL_MAP == 1
	 vec3 N = normalFromMap();
	#endif

	// Get specular parameters
	#if SPECULAR == 1
		vec2 specParamSample = texture(specParamTex, vTextureCoordinate).rg;
		float rho_s = specParamSample.r;
	    float roughness = specParamSample.g;
	#endif 

	// view vector
	vec3 V = normalize(V);

	#if (HEMOGLOBINE != 0 && MELANINE != 0)
	vec3 diffuseColor =	skinColour();
	#else
	vec3 diffuseColor = texture(diffuseTexture, vTextureCoordinate).rgb;
	#endif

	#if GAMMA == 1
	diffuseColor = pow(diffuseColor, vec3(2.2)); // A pow(1/2.2) will be performed in tonemapper
	#endif




	vec3 Ldist = lights[lightIndex].position.xyz - worldSpacePosition.xyz;
	float d = length(Ldist);
	// Light vector
	vec3 L = normalize(Ldist); 
	// For diffuse component
	float attenuation = computeAttenuation(lights[lightIndex],d);
	attenuation = 1.0;											// testing without attenuation
	float NdotL = max(dot(N,L),0.0);
	vec4 diffuseLightingContribution = NdotL*lights[lightIndex].intensity*attenuation;

	// Specular component
	vec4 specularLightingContribution = diffuseLightingContribution*computeBRDF(roughness, L, N, V);

	diffuseLightingContribution /= PI;

// Compute the visibility for current pixel if shadows are actie
#if SHADOW == 1
float visibility = visibility();
#else
float visibility = 1.0;
#endif


// The output if no particular output flags are activated. It will be overwritten in case of special options
fragmentColor.rgb = (1.0-rho_s)*visibility*diffuseLightingContribution.rgb*diffuseColor + rho_s*specularLightingContribution.rgb;
fragmentColor.a = 1.0;

#if SPEC_ONLY == 1
fragmentColor = vec4(specularLightingContribution.rgb, 1.0);
#endif

#if DEBUG_NO_COL == 1
fragmentColor.rgb = (1.0-rho_s)*visibility*diffuseLightingContribution + rho_s*specularLightingContribution.rgb;
#endif

#if DEBUG_SHOW_NORMAL == 1
fragmentColor = vec4(N.rgb,1.0);
#endif

#if JUST_COLOR == 1
fragmentColor = vec4(diffuseColor.rgb,1.0);
#endif 


#if NO_SPEC == 1
fragmentColor.rgb = visibility*diffuseLightingContribution*diffuseColor;
#endif

fragmentColor.rgb = (1.0-rho_s)*visibility*diffuseLightingContribution.rgb*diffuseColor + rho_s*specularLightingContribution.rgb;
fragmentColor.a = 1.0;

}


