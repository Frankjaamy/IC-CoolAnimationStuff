#version 430

layout(location = 0) in vec3 ex_worldPos;
layout(location = 1) in vec3 ex_Normal;
layout(location = 2) in vec2 ex_TexCoord0;
layout(location = 3) in vec4 shadowCoord;

layout(location = 0) out vec4 gl_FragColor;

uniform vec3 viewPosition;
uniform vec3 lightDirection;

uniform sampler2D texSampler0;
uniform sampler2D texSampler1;
uniform sampler2D shadowMap;


void main(void) {
	// Pass through our original color with full opacity.

	vec3 _diffuseColor = vec3(texture2D(texSampler0, ex_TexCoord0.st));
	vec3 _specularColor = vec3(texture2D(texSampler1, ex_TexCoord0.st));
	
	vec3 _ambientColor = vec3(0.5, 0.1, 0.1);
	float specularPower = 20;

	vec3 lightDir = normalize(-lightDirection);
	vec3 normal = normalize(ex_Normal);
	float diffuseIntense = clamp(dot(lightDir, normal), 0.0, 1.0);

	vec3 eyeDir = normalize(viewPosition - ex_worldPos);
	vec3 halfLightEye = normalize((lightDir + eyeDir) * 0.5);
	float specularIntense = pow(clamp(dot(halfLightEye, normal), 0.0, 1.0), specularPower);

	vec3 _shadowColor = vec3(texture2D(shadowMap,ex_TexCoord0.st));

	float visibility = 1.0;
	if ( texture( shadowMap, shadowCoord.xy ).z  <  shadowCoord.z - 0.15){
		visibility = 0.5;
	}

	//gl_FragColor = vec4(_diffuseColor,1.0f);
	gl_FragColor = visibility * vec4(_diffuseColor * diffuseIntense,1.0f) 
	    + visibility * vec4(_specularColor * specularIntense,1.0f) 
		+ vec4(_ambientColor, 1.0);
}