#version 410 core

layout(location=0) in vec3 inPosition;
layout(location=2) in vec2 inTexCoord;

out vec2 texCoord;

void main( void ) {
	texCoord = inTexCoord;
	gl_Position = vec4(inPosition, 1);
}