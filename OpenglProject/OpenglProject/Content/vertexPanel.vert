#version 430
// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;

layout(location = 0) out vec4 shadowCoord;
// Values that stay constant for the whole mesh.
uniform mat4 depthMVP;

void main(void){
	gl_Position =  depthMVP * vec4(vertexPosition_modelspace,1);
}