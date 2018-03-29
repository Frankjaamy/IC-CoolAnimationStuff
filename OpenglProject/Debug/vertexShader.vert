#version 430
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoord0;



// We output the ex_Color variable to the next shader in the chain
layout(location = 0) out vec3 ex_worldPos;
layout(location = 1) out vec3 ex_Normal;
layout(location = 2) out vec2 ex_TexCoord0;
layout(location = 3) out vec4 shadowCoord;

uniform mat4 cameraMatrix;
uniform mat4 biasMVP;

void main(void) {
	vec4 newPosition = vec4(in_Position,1.0);
	gl_Position = cameraMatrix * newPosition;


	ex_worldPos = in_Position;
	ex_Normal = in_Normal;
	ex_TexCoord0 = in_TexCoord0;
	shadowCoord = biasMVP * newPosition;
}