#version 330

layout(location=0) in vec3 aVertexPosition;

uniform mat4 mvpMatrix;

void main(void){
	gl_Position = mvpMatrix*vec4(aVertexPosition,1.0);
}