//#version 410 core
//
//layout( location = 0 ) in vec3 vertexPosition;
//
//uniform mat4 shadowBiasMVP;
//uniform mat4 shadowMVP;
//
//out vec4 shadowCoord;
//
//void main( void ){
//	shadowCoord = shadowBiasMVP * vec4(vertexPosition, 1.0f);
//
//	gl_Position = shadowMVP * vec4(vertexPosition, 1.0f);
//}


#version 410 core

layout( location = 0 ) in vec3 vertexPosition;

// uniform mat4 shadowBiasMVP;
// uniform mat4 shadowMVP;

out vec4 shadowCoord;

uniform mat4 modelMat = mat4(1);
uniform mat4 viewMat;
uniform mat4 projMat;

uniform mat4 shadowProjMat;
uniform mat4 shadowViewMat;
out float shadowDepth;


void main( void ) {
	vec4 world_Pos = modelMat * vec4( vertexPosition, 1. );
	vec4 shadow_Pos = shadowViewMat * world_Pos;
	shadowDepth = shadow_Pos.z;
	shadowCoord = shadowProjMat * shadow_Pos;
	// gl_Position = projMat * viewMat * world_Pos;
	gl_Position = shadowCoord;
}