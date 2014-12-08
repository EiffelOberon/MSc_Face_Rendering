#version 330


#define SAMPLE_NUMBER 7
#define PIXELSIZE

in vec2 vTextureCoordinate;

uniform float gaussWidth;
uniform sampler2D colTexture; 
uniform vec2 direction; // X is (1,0), Y is (0,1)

out vec4 fragmentColor;


#if SAMPLE_NUMBER == 5
	const float weights[5] = float[](0.0545, 0.2442, 0.4026, 0.2442, 0.0545);
	const int offsets[5] = int[](-2, -1, 0, 1, 2);
#elif SAMPLE_NUMBER == 7
	const float weights[7] = float[](0.006, 0.061, 0.242, 0.382, 0.242, 0.061, 0.006);
	const int offsets[7] = int[](-3, -2, -1, 0, 1, 2, 3);
#elif SAMPLE_NUMBER == 9
	const float weights[9] = float[](0.0001, 0.0044, 0.0540, 0.2420, 0.3989, 0.2420, 0.0540, 0.0044, 0.0001);
	const int offsets[9] = int[](-4, -3, -2, -1, 0, 1, 2, 3, 4);
#endif


void main(void){

	vec2 width = direction*gaussWidth*pixelSizes;
	fragmentColor = vec4(0.0);
	for(int i=0; i<SAMPLE_NUMBER; i++){
		fragmentColor += texture(colTexture,vTextureCoordinate + offsets[i]*width)*weights[i];
	}

	fragmentColor = vec4(fragmentColor.rgb, 1.0);
}

