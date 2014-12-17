#version 330

/**********************************
* 		SSSSS Blur 			  *
*								  *
***********************************/


// from vertex shader
in vec2 vTextureCoordinate;

// Gaussian Filter related
const float weights[7] = float[](0.006, 0.061, 0.242, 0.382, 0.242, 0.061, 0.006);
//const float offsets[7] = float[](-3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0);

uniform float gaussWidth;

// Texture datas
uniform sampler2D colourTexture;
uniform sampler2D depthTexture;
uniform vec2 pixelSizes;
uniform vec2 direction;

// Mesh MVP to unproject
uniform mat4 invMVP; 

out vec4 fragmentColour;

// Convert screen space to obj space
vec3 getObjectCoord(vec2 uv, float depth){
	vec3 ndc = 2.0*vec3(uv,depth) - 1.0;
	vec4 objSpace = invMVP * vec4(ndc,1.0);
	objSpace = objSpace / objSpace.w;
	return objSpace.xyz;
}

// Get distance between the object space point corresponding to central sample
// and the one that in screen space is distant "offset" from the central sample
float getDist(vec3 objCentral, vec2 offset){
	vec2 tapUV = vTextureCoordinate + offset;
	float tapDepth = texture(depthTexture, tapUV).r;
	vec3 tapObj = getObjectCoord(tapUV, tapDepth);
	float dist = length(objCentral - tapObj);
	dist = dist*1000.0f; // Assumes meshes defined in metres. 
	return dist;
}


// Get a direction in screen space such as the corresponding vector is perpendicular in object
// space to the one used for the first blur pass (i.e. X direction in screen space).
// This is achieved via Gram-Schmidt
vec3 getDirection(vec3 objCentral, float  centralDepth){

        
	vec2 xTap = vTextureCoordinate + vec2(1.0,0.0)*pixelSizes.x;	
	vec2 yTap = vTextureCoordinate + vec2(0.0,1.0)*pixelSizes.y;
	float yDepth = texture(depthTexture, yTap);
	float xDepth = texture(depthTexture, xTap);
	
	
	vec3 yObj = getObjectCoord(yTap, yDepth);
	vec3 xObj = getObjectCoord(xTap, xDepth);

	// Forward differences in obj space
	vec3 xVec = normalize(xObj  - objCentral);
	vec3 yVec = (yObj - objCentral);
	float dist = length(yVec);
	yVec = normalize(yVec);

	// Gram-Schmidt
	vec3 perpDir = normalize(yVec - dot(yVec, xVec) * xVec);

        // Transform back to screen space a point from obj space taken in the new directio.
	mat4 mvpMatrix = inverse(invMVP);         // Evaluate passing directly MVP. 
	vec3 tapObj = objCentral + dist*perpDir;	// Take the obj-space point in the new direction, distant "dist", from the central point

        
	vec4 screenSpacePoint = vpMatrix*vec4(tapObj,1.0);
	vec3 projPoint = screenSpacePoint.xyz/screenSpacePoint.w;
	vec3 newPos = 0.5 + projPoint*0.5;

	// Get direction in screen space
	return (newPos - vec3(vTextureCoordinate, centralDepth));


}


void main(void){

	fragmentColour = vec4(0.0);	

	float centralDepth = texture(depthTexture, vTextureCoordinate).r;
	vec3 centralObjSpace = getObjectCoord(vTextureCoordinate, centralDepth);

	vec2 offset = direction*pixelSizes.y;

	float D = getDist(centralObjSpace, offset);
	float stepSize = gaussWidth/D; // offset so that it correspond in OBJ SPACE! to sigma mm 

	vec2 newStep;  
	if(direction.y == 1.0){		// i.e. is the second pass. Previously was another shader. 
		vec2 correctedDir = getDirection(centralObjSpace, centralDepth).xy;
		newStep = correctedDir * stepSize;
	}else{
		newStep = pixelSizes * direction * stepSize;  
	}


	// -3*sigma/3
	offset = vTextureCoordinate + newStep*-3.0;
	vec3 tap = texture(colourTexture, offset).rgb;
	fragmentColour.rgb += weights[0]*tap;

	// -2*sigma/3
	offset = vTextureCoordinate + newStep*-2.0;
	tap = texture(colourTexture, offset).rgb;
	fragmentColour.rgb += weights[1]*tap;

	// -1*sigma/3
	offset = vTextureCoordinate + newStep*-1.0;
	tap = texture(colourTexture, offset).rgb;
	fragmentColour.rgb += weights[2]*tap;

	// 0 
	fragmentColour.rgb += weights[3]*texture(colourTexture, vTextureCoordinate).rgb;

	// sigma/3
	offset = vTextureCoordinate + newStep*1.0;
	tap = texture(colourTexture, offset).rgb;
	fragmentColour.rgb += weights[4]*tap;

	// 2*sigma/3
	offset = vTextureCoordinate + newStep*2.0;
	tap = texture(colourTexture, offset).rgb;
	fragmentColour.rgb += weights[5]*tap;

	// 3*sigma/3
	offset = vTextureCoordinate + newStep*3.0;
	tap = texture(colourTexture, offset).rgb;
	fragmentColour.rgb += weights[6]*tap;


	fragmentColour.a = 1.0;

}


