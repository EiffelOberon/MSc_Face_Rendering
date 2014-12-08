#version 330

layout(location=0) in vec3 aVertexPosition;
out vec2 vTextureCoordinate;

void main(void){
	gl_Position = vec4(aVertexPosition,1.0);
	vTextureCoordinate = (aVertexPosition.xy+vec2(1,1))/2.0;
}
