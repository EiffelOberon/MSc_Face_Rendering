#version 330

/**********************************
* 		SSSSS Blur		
*					  
*	This is the Jorge Jimenez work 
*								  
***********************************/

// from vertex shader
in vec2 vTextureCoordinate;

// Gaussian Filter related
const float weights[7] = float[](0.006, 0.061, 0.242, 0.382, 0.242, 0.061, 0.006);
const float offsets[7] = float[](-3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0);

uniform float gaussWidth;
uniform vec2 direction;

// Texture datas
uniform sampler2D colourTexture;
uniform sampler2D depthTexture;

// SSSS Params
uniform float sssLevel;
uniform float correction;
uniform float maxdd;

// Generic
uniform vec2 pixelSizes;

out vec4 fragmentColour;




void main(void){

	vec3 centralColour = texture(colourTexture, vTextureCoordinate).rgb;
	float centralDepth = texture(depthTexture, vTextureCoordinate).r;
	fragmentColour.rgb = vec3(0.0);

	vec2 tapStep = sssLevel * gaussWidth * pixelSizes * direction / ( 3.0f * centralDepth);	

	for(int i=0; i<7; i++){
		
		vec2 offset = vTextureCoordinate + offsets[i] * tapStep;
		vec3 tapColour = texture(colourTexture, offset).rgb;
		float tapDepth = texture(depthTexture, offset).r;

		float lerpWeigth = min(0.0125 * correction * abs(centralDepth - tapDepth), 1.0);
		tapColour = mix(tapColour, centralColour, lerpWeigth);

		fragmentColour.rgb += weights[i] * tapColour;
	}


	fragmentColour.a = 1.0;
}