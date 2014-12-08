#version 330

in vec4 vPosition;
out vec4 fragmentColor;

// Partial reference: http://http.developer.nvidia.com/GPUGems3/gpugems3_ch08.html

void main(void){
	float depth = vPosition.z / vPosition.w;
	// (1 + depth)/2. Moving to [0,1] 
	depth = 0.5 + depth*0.5;
	float dx = dFdx(depth);
	float dy = dFdy(depth);

	float firstMoment = depth;
	float secondMoment = depth*depth + 0.25*(dx*dx + dy*dy);

	fragmentColor = vec4(firstMoment, secondMoment, 0.0, 1.0);
}