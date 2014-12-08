/********************************************************************
*			      ToneMapping Frag shader 0.1						*
*	Note: the shader assume that the input texture, colTexture,		*
*	is a floating point texture (for my application was a RGBA16F)
*	Some tonemapping techniques are available at the moment. These
	are selected through TONE_MAPPER. According to its value a 
	function is selected. If: 
		- 0 : Plain pow to 1/2.2
		- 1 : Exponential 
		- 2 : Reinhard
		- 3 : Jim Hejl and Richard Burgess-Dawson
		- 4 : Uncharted 2 tone mapping
*																	*
*																	*
********************************************************************/


#version 330

#define TONE_MAPPER 3

in vec2 vTextureCoordinate;
out vec4 fragmentColor;

uniform sampler2D colTexture; 
uniform float exposure;



#if TONE_MAPPER == 0
vec4 gammaCorrection(vec3 color){
	color *= exposure;
	// Hard coded 1/2.2
	color = pow(color, vec3(0.4545));
	return vec4(color, 1.0);
}
#endif

#if TONE_MAPPER == 1
vec4 exponential(vec3 color){
	color = 1.0  - exp2(-exposure*color);
	color = pow(color, vec3(0.4545));	
	return vec4(color, 1.0);
}
#endif

#if TONE_MAPPER == 2
vec4 reinhard(vec3 color){
	color *= exposure;
	color = color / (1+color);
	color = pow(color, vec3(0.4545));		
	return vec4(color, 1.0);
}
#endif

#if TONE_MAPPER == 3
vec4 hejl_dawson(vec3 color){
	color *= exposure;
	color = max(vec3(0.0), color-0.004);
	color = (color*(6.2*color + 0.5)) / (color*(6.2*color+1.7)+0.06);
	return vec4(color,1.0);
}
#endif

#if TONE_MAPPER == 4


vec3 U2ToneMap(vec3 x){
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;	

	return ((x*(A*x + C*B)+D*E) / (x*(A*x+B)+D*F)) - E/F;
}

vec4 hable(vec3 color){
	color *= exposure;
	float exposureBias = 2.0f;
	color = U2ToneMap(exposureBias*color);
	vec3 whiteScale = vec3(1.0f)/U2ToneMap(vec3(11.2)); // Note this 11.2 is the W above
	color = color*whiteScale;
	color = pow(color, vec3(0.4545));
	return vec4(color,1.0);
}

#endif

vec4 toneMap(){
	vec3 color = texture(colTexture, vTextureCoordinate).rgb;

	#if TONE_MAPPER == 0
		return gammaCorrection(color);
	#elif TONE_MAPPER == 1

		return exponential(color);
	#elif TONE_MAPPER == 2

		return reinhard(color);
	#elif TONE_MAPPER == 3

		return hejl_dawson(color);
	#elif TONE_MAPPER == 4	
		return hable(color);
	#endif
}


void main(void){
	fragmentColor = toneMap();
}
