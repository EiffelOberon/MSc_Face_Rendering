/********************************************************************
 	Note: this is a skeleton for the shader. The lines relative to 
 	the number of textures and the weights to use is added to the
 	source while creating the appropriate shader. 
 	The generated lines can be for example (weights are wrong):
 		//const int textureCount = 6;
		//const float weights[6] = float[](1,2,3,4,5,6);
		//uniform sampler2D textures[6]
********************************************************************/

#version 330
 // The following defines are not part of the algorithm, nor are set by the applications.  They are manually set if debug has to be done on single images.

 // If set shows just DEBUG_IMAGE-th image 
#define DEBUG_SINGLE_IMAGE 0
// If set show just weights for DEBUG_IMAGE
#define DEBUG_WEIGHTS 0

 #define DEBUG_IMAGE 0

// from vertex shader
in vec2 vTextureCoordinate;


out vec4 fragmentColour;

void main(void){

	fragmentColour = vec4(0.0);	
	for(int i=0; i<textureCount; i++){
		vec4 currentSample = texture(textures[i], vTextureCoordinate);
		// Sum the various textures with right weights
		fragmentColour += vec4(currentSample.r * weights[(3*i)], currentSample.g * weights[(3*i)+1], currentSample.b * weights[(3*i)+2],1.0);
	}
	fragmentColour = vec4(fragmentColour.rgb,1.0);

#if DEBUG_SINGLE_IMAGE == 1 && DEBUG_WEIGHTS == 1
	vec4 currentSample = texture(textures[DEBUG_IMAGE], vTextureCoordinate);
	fragmentColour = vec4(currentSample.r * weights[(3*DEBUG_IMAGE)], currentSample.g * weights[(3*DEBUG_IMAGE)+1], currentSample.b * weights[(3*DEBUG_IMAGE)+2],1.0);
#elif DEBUG_SINGLE_IMAGE == 1
 	fragmentColour = texture(textures[DEBUG_IMAGE], vTextureCoordinate);

#endif

#if DEBUG_WEIGHTS == 1 && DEBUG_SINGLE_IMAGE == 0
	fragmentColour = vec4(weights[(3*DEBUG_IMAGE)], weights[(3*DEBUG_IMAGE)+1], weights[(3*DEBUG_IMAGE)+2],1.0);
#endif

}