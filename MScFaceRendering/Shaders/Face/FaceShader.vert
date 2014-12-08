/******************************************
*			Skin Vert Shader 0.2		  *
*										  *
*******************************************/

#define PI 3.14159265359f


#version 400 

// 0 false 1 true
#define NORMAL_MAP 1
#define SHADOW 1

// Vertex attributes
layout(location=0) in vec3 aVertexPosition;
layout(location=1) in vec2 aTextureCoordinate;
layout(location=2) in vec3 aVertexNormal;
#if NORMAL_MAP == 1
layout(location=3) in vec3 aVertexTangent;
#endif


uniform mat4 vMatrix;
uniform mat4 mMatrix;
uniform mat4 pMatrix;
uniform mat3 nMatrix;

uniform vec3 cameraPosition;

// Outputs
out vec2 vTextureCoordinate;
out vec4 worldSpacePosition;
#if NORMAL_MAP == 1
out vec3 t;
out vec3 n;
out vec3 b;
#else
out vec3 vVertexNormal;
#endif
out vec3 V;




#if SHADOW == 1
out vec4 shadowCoordinates;	
uniform mat4 depthVP;
#endif


out vec3 test;

void main(void){

	#if NORMAL_MAP == 1
		// Here mMatrix is mat3 to avoid translation component. 
		t = (mMatrix*vec4(aVertexTangent, 0.0)).xyz;
		n = (mMatrix*vec4(aVertexNormal, 0.0)).xyz;
		b = (mMatrix*vec4(cross(aVertexTangent,aVertexNormal),0.0)).xyz;
	#else
		vVertexNormal = nMatrix*aVertexNormal;
	#endif



	worldSpacePosition = mMatrix * vec4(aVertexPosition,1.0);
	V =  cameraPosition - worldSpacePosition.xyz;
	vTextureCoordinate = aTextureCoordinate;

	gl_Position = pMatrix*vMatrix * worldSpacePosition;

	#if SHADOW == 1
		shadowCoordinates = depthVP * mMatrix *  vec4(aVertexPosition,1.0);
	#endif 

}