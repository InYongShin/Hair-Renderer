
#version 410 core

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;

uniform mat4 modelMat = mat4(1);
uniform mat4 viewMat;
uniform mat4 projMat;
// uniform mat4 shadowBiasMVP;

out vec4 shadowCoord;
out vec3 worldPos;
out vec3 normal;
out float shadowDepth;

uniform mat4 shadowProjMat;
uniform mat4 shadowViewMat;

void main( void ) {
	// shadowCoord = shadowBiasMVP * vec4(inPosition, 1);
	vec4 world_Pos = modelMat * vec4(inPosition, 1);
	worldPos = world_Pos.xyz;
	normal = (modelMat * vec4(inNormal, 0)).xyz;

	vec4 shadow_Pos = shadowViewMat * (world_Pos /*+vec4(normal*0.0001f,0)*/ );
	shadowDepth = shadow_Pos.z;
	shadowCoord = shadowProjMat * shadow_Pos;

	gl_Position = projMat * viewMat * world_Pos;
}