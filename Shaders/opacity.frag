#version 410 core 

uniform sampler2D depthMap;

uniform float layerSize;
uniform float opacityValue;

uniform int mode = 0;

in vec4 shadowCoord;
in float shadowDepth;

out vec4 opacity;


void main( void ){
	vec2 shadowUV = (shadowCoord.xy / shadowCoord.w)*0.5f + 0.5f;
	float z = texture(depthMap, shadowUV).r + 0.0001f;

	float layer1Z = z		+  layerSize;
	float layer2Z = layer1Z + (layerSize * 2);
	float layer3Z = layer2Z + (layerSize * 3);
	float layer4Z = layer3Z + (layerSize * 4);

	 float d = (shadowCoord.z / shadowCoord.w)*0.5 + 0.5;
// 	float d = shadowDepth;
//	if( d > z + 0.00001f ) {
//		// TODO: Test 
//		opacity = vec4(0);
//	}
//	else
	if( d < layer1Z ) {
		opacity = vec4(opacityValue, opacityValue, opacityValue, opacityValue);
		// opacity = vec4(1,0,0,1);
		// opacity.r = opacityValue;
	}
	else if( d < layer2Z ) {
		opacity = vec4(0, opacityValue, opacityValue, opacityValue);
		// opacity = vec4(0,1,0,1);
		// opacity.g = opacityValue;
	}
	else if( d < layer3Z ) {
		opacity = vec4(0, 0, opacityValue, opacityValue);
		// opacity = vec4(0,0,1,1);
		// opacity.b = opacityValue;
	}
	else{
		opacity = vec4(0, 0, 0, opacityValue);
	}
//	else if( d < layer4Z ) {
//		opacity = vec4(0, 0, 0, opacityValue);
//		// opacity = vec4(1,1,0,1);
//		// opacity.a = opacityValue;
//	}
//	else {
//		opacity = vec4(0);
//	}
}
