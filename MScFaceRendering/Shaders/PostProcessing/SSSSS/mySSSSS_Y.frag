#version 330

/**********************************
* 		SSSSS Blur Y			  *
* Note: Use the mySSSSS.frag shader instead. Here passes are
* separated in case two separate shader program are desired, but
* I can't really see an actual usage of these over mySSSSS.frag
***********************************/

// Todo: better commnets

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


// Mesh MVP to unproject
uniform mat4 invMVP; 

out vec4 fragmentColour;


vec3 getObjectCoord(vec2 uv, float depth){

	vec3 ndc = 2.0*vec3(uv,depth) - 1.0;
	vec4 objSpace = invMVP * vec4(ndc,1.0);
	objSpace = objSpace / objSpace.w;
	return objSpace.xyz;
}


float getDist(vec3 objCentral, vec2 offset){
	vec2 tapUV = vTextureCoordinate + offset;
	float tapDepth = texture(depthTexture, tapUV).r;
	vec3 tapObj = getObjectCoord(tapUV, tapDepth);
	float dist = length(objCentral - tapObj);
	dist = dist*1000.0f; // Assumes meshes defined in metres. 
	return dist;
}


vec3 getDirection(vec3 objCentral){

	vec2 xTap = vTextureCoordinate + vec2(1.0,0.0)*pixelSizes.x;	
	vec2 yTap = vTextureCoordinate + vec2(0.0,1.0)*pixelSizes.y;
	float yDepth = texture(depthTexture, yTap);
	float xDepth = texture(depthTexture, xTap);

	vec3 yObj = getObjectCoord(yTap, yDepth);
	vec3 xObj = getObjectCoord(xTap, xDepth);

	vec3 xVec = normalize(xObj  - objCentral);
	vec3 yVec = normalize(yObj - objCentral);

	vec3 perpDir = normalize(yVec - dot(yVec, xVec) * xVec);

	mat4 vpMatrix = inverse(invMVP);


	float tapDepth = texture(depthTexture, yTap).r;
	vec3 tapObj = getObjectCoord(yTap, tapDepth);
	float dist = length(tapObj - objCentral);
	vec3 step_ = normalize(tapObj - objCentral);
	vec3 otherTest = objCentral + dist*step_;	
	tapObj = objCentral + dist*perpDir;


	vec4 test = vpMatrix*vec4(tapObj,1.0);
	vec3 projTest = test.xyz/test.w;
	vec3 outTest = 0.5 + projTest*0.5;

return (outTest - vec3(vTextureCoordinate, texture(depthTexture, vTextureCoordinate)));


}


void main(void){

	fragmentColour = vec4(0.0);	
	vec4 centralColor = texture(colourTexture, vTextureCoordinate);
	float centralDepth = texture(depthTexture, vTextureCoordinate).r;
	
	vec3 centralObjSpace = getObjectCoord(vTextureCoordinate, centralDepth);
	vec2 offset = vec2(0.0,1.0)*pixelSizes.y;

	float a = getDist(centralObjSpace, offset);

	float b = gaussWidth/a; // offset so that it correspond in OBJ SPACE! to sigma mm 

	vec3 dirr = getDirection(centralObjSpace);
	vec2 direction = dirr.xy;


	// -3*sigma/3
	offset = vTextureCoordinate + direction*-3.0*b;
	vec3 tap = texture(colourTexture, offset).rgb;
	fragmentColour.rgb += weights[0]*tap;

	// -2*sigma/3
	offset = vTextureCoordinate + direction*-2.0*b;
	tap = texture(colourTexture, offset).rgb;
	fragmentColour.rgb += weights[1]*tap;

	// -1*sigma/3
	offset = vTextureCoordinate + direction*-1.0*b;
	tap = texture(colourTexture, offset).rgb;
	fragmentColour.rgb += weights[2]*tap;

	// 0 
	fragmentColour.rgb += weights[3]*centralColor.rgb;

	// sigma/3
	int i=0;
	offset = vTextureCoordinate + direction*1.0*b;
	tap = texture(colourTexture, offset).rgb;
	fragmentColour.rgb += weights[4]*tap;



	// 2*sigma/3
	offset = vTextureCoordinate + direction*2.0*b;
	tap = texture(colourTexture, offset).rgb;
	fragmentColour.rgb += weights[5]*tap;

	// 3*sigma/3
	offset = vTextureCoordinate + direction*3.0*b;
	tap = texture(colourTexture, offset).rgb;
	fragmentColour.rgb += weights[6]*tap;


	fragmentColour.rgb = centralColor.rgb;
	fragmentColour.a = 1.0;

}


