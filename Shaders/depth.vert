//#version 410 core
//
//layout( location = 0 ) in vec3 vertexPosition;
//uniform mat4 shadowMVP;
//
//void main(void){
//	gl_Position = shadowMVP * vec4(vertexPosition, 1.0f);
//}

#version 410 core

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;

uniform mat4 modelMat = mat4(1);
uniform mat4 viewMat;
uniform mat4 projMat;

out vec3 lightPos;

void main( void ) {
	vec4 world_Pos = modelMat * vec4( inPosition, 1. );
	vec4 light_Pos = viewMat * world_Pos;
	lightPos = light_Pos.xyz;
	gl_Position = projMat * light_Pos;
}