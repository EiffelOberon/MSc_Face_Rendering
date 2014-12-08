

#version 400

// 0 false 1 true
#define NORMAL_MAP 1
#define SHADOW 1

                                                                                                
layout(location=0) in vec3 aVertexPosition;
layout(location=1) in vec2 aTextureCoordinate;
layout(location=2) in vec3 aVertexNormal;
#if NORMAL_MAP == 1
layout(location=3) in vec3 aVertexTangent;
#endif


uniform mat4 mMatrix;
uniform mat3 nMatrix;
                                    
uniform vec3 cameraPosition;

out vec3 worldSpacePosition_CS;
out vec2 vTextureCoordinate_CS;


#if NORMAL_MAP == 1
// Tangent basis to Tessellation control shader
out vec3 t_CS;
out vec3 n_CS;
out vec3 b_CS;
#else
out vec3 vVertexNormal_CS;
#endif

#if SHADOW == 1
out vec4 shadowCoordinates_CS;	
uniform mat4 depthVP; // View-projection matrix for the "light-camera"
#endif

out vec3 V_CS; // View vector
                                                          
void main()                                                                                     
{
	#if NORMAL_MAP == 1
		t_CS = (mMatrix*vec4(aVertexTangent,0.0)).xyz;
		n_CS = (mMatrix*vec4(aVertexNormal, 0.0)).xyz;
		b_CS = (mMatrix*vec4(cross(aVertexTangent,aVertexNormal),0.0)).xyz;
	#else
		vVertexNormal_CS = nMatrix*aVertexNormal;
	#endif

	worldSpacePosition_CS = (mMatrix * vec4(aVertexPosition,1.0)).xyz;
    vTextureCoordinate_CS = aTextureCoordinate;       

    // View vector to curent vertex
    V_CS =  cameraPosition - worldSpacePosition_CS;                             

    #if SHADOW == 1
    shadowCoordinates_CS = depthVP * mMatrix * vec4(aVertexPosition,1.0);
    #endif
}
