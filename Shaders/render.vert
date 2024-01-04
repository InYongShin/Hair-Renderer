
#version 410 core

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inTangent;

uniform mat4 modelMat = mat4(1);
uniform mat4 viewMat;
uniform mat4 projMat;
// uniform mat4 shadowBiasMVP;

out vec4 shadowCoord;
out vec3 worldPos;
out vec3 tangent;

uniform mat4 shadowProjMat;
uniform mat4 shadowViewMat;
out float shadowDepth;

void main( void ) {
	// shadowCoord = shadowBiasMVP * vec4(inPosition, 1.0f);
	vec4 world_Pos = modelMat * vec4(inPosition, 1);
	worldPos = world_Pos.xyz;
	tangent = (modelMat * vec4(inTangent, 0)).xyz;

	vec4 shadow_Pos = shadowViewMat * world_Pos;
	shadowDepth = shadow_Pos.z;
	shadowCoord = shadowProjMat * shadow_Pos;

	gl_Position = projMat * viewMat * world_Pos;
}