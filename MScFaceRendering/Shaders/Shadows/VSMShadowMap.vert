#version 330

layout(location=0) in vec3 aVertexPosition;

out vec4 vPosition;

uniform mat4 mvpMatrix;

void main(void){
	vPosition = mvpMatrix*vec4(aVertexPosition,1.0);	
	gl_Position = vPosition;
} 