
#version 410 core

in vec3 lightPos;

out vec4 out_Color;

void main( void ){
	out_Color = vec4(gl_FragCoord.z, lightPos.z, 0, 1);

	// out_Color = vec4(-3.5f, 0, 0, 1);
}