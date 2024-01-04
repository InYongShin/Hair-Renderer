
#version 410 core

const float pi = 3.141592;
const float GOLDEN_ANGLE = 2.39996;

out vec4 outColor;

in vec4 shadowCoord;
in float shadowDepth;
in vec3 worldPos;
in vec3 normal;

uniform vec4 albedo;

uniform vec3 camPos;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform float lightFactor;
uniform float lightSize = 1.;

uniform float roughness;

uniform float layerSize;

uniform float shadowBias;

uniform sampler2D depthMap;
uniform sampler2D opacityMap;
uniform sampler2D headDepthMap;

uniform int Pass = 0;

uniform float type = 0;


vec3 FresnelSchlick( float cosTheta, vec3 F0 ) {
	return F0 + (1-F0)*pow((1-cosTheta),5);
}

float GGX( float cosTheta_, float roughness ) {
	float alpha2 = pow(roughness, 4);
	float cosTheta = clamp( cosTheta_, 0.00001, 1. );
	float theta = acos(cosTheta);
	float tanTheta = tan(theta);
	return alpha2/(pi*pow(cosTheta,4) * pow(alpha2+pow(tanTheta,2),2));
}

float ShlickGeometry( vec3 v, vec3 N, float roughness ) {
	float cosTheta = clamp(dot(N,v), 0.00001, 1.);
	float alpha = roughness * roughness;
	float k = sqrt( (2*alpha)/pi );
	return cosTheta/(cosTheta*(1-k)+k);
}

float SmithGeometry( vec3 w_i, vec3 w_o, vec3 N, float roughness ) {
	return ShlickGeometry(w_i, N, roughness) * ShlickGeometry(w_o, N, roughness);
}

vec3 specularPoint( vec3 N, vec3 L, vec3 V, vec3 abd, float roughness){
	vec3 eta2 = vec3(1.55*1.55);

	vec3 H = normalize( V + L );
	float D = GGX(dot(H,N), roughness);
	vec3 F = FresnelSchlick( clamp(dot(H,N),0,1), vec3(0.04) );
	float G = SmithGeometry(L,V,N,roughness);
	float denorm = max(dot(V,N),0.01);
	return max(vec3(0), D*G*F *.25f/denorm);
}


vec2 VogelDiskSample(int k, int n, float offset){
	float r = sqrt(float(k)+0.5) / sqrt(float(n));
	float theta = float(k) * GOLDEN_ANGLE + offset;
	return r * vec2(cos(theta),sin(theta) );
}

float rand(){
	float d = dot(gl_FragCoord.xy, vec2(12.9898f, 78.233f));
	return fract(sin(d) * 43759.5453f);
}

float PCF( sampler2D shadowMap, vec2 shadowUV, float shadowDepth, float radius ) {
	float r_offset = rand()*2*pi;
	float shadowFactor = 1;
	
	for(int k=0; k<64; k++){
		if( texture(shadowMap, shadowUV + VogelDiskSample(k, 64, r_offset) * radius).r > shadowDepth ) {
			shadowFactor -= 1/64.f;
		}
	}
	return shadowFactor;
}

float PCSS( sampler2D shadowMap, vec2 shadowUV, float shadowDepth, float lightSize ){
	float depthSum = 0;
	float numBlockers = 0;

	float r_offset = rand()*2*pi;
	for(int k=0; k<4; k++){
		float dd = texture(shadowMap, shadowUV + VogelDiskSample(k, 4, r_offset) * 0.01*lightSize).r;
		if( dd > shadowDepth ) {
			depthSum += dd;
			numBlockers += 1;
		}
	}
	if( numBlockers<1 ) return 1;
	float blockerDepth = depthSum/numBlockers;
	float w_penumbra = (-shadowDepth + blockerDepth) * lightSize / -blockerDepth;
	return PCF(shadowMap, shadowUV, shadowDepth, w_penumbra * 0.01f);
}


void main( void ) {

//	int nSample = 10;
//	float bias = 0.000005f;
//	float shadow = 0.0;
//	vec2 texelSize = 1.0 / textureSize(depthMap, 0);
//	for(int x = -nSample; x<=nSample; x++){
//		for(int y = -nSample; y<=nSample; y++){
//			float pcfDepth = texture(depthMap, sCoord.xy + vec2(x,y) * texelSize).r;
//			shadow += sCoord.z - bias > pcfDepth ? 1.0f : 0.0;
//		}
//	}
//	shadow /= nSample*nSample;
//	visibility = 1 - shadow;


//	if(type < 0.5) {
//		vec2 shadowUV = (shadowCoord.xy / shadowCoord.w)*0.5f + 0.5f;
//		float z = texture(depthMap, shadowUV).r + 0.003f;
//		outColor = vec4(vec3(z), 1);
//	}
//	else {
//		float d = (shadowCoord.z / shadowCoord.w)*0.5f + 0.5f;
//		outColor = vec4(vec3(d), 1);
//	}
//	return;


//	vec2 size = textureSize(depthMap, 0); // Size of texture (e.g. 1024, 1024)
//  vec2 texelSize = vec2(1.) / size; // Size of texel (e.g. 1/1024, 1/1024)
//	ivec2 offset = ivec2(0, 1);

	// Depth
	vec2 shadowUV = (shadowCoord.xy / shadowCoord.w)*0.5f + 0.5f;
	// float z = texture(hairDepthMap, shadowUV).r;

	float z = texture(depthMap, shadowUV).r + shadowBias;
	float layer1Z = z		+   layerSize;
	float layer2Z = layer1Z + ( layerSize * 2 );
	float layer3Z = layer2Z + ( layerSize * 3 );
	float layer4Z = layer3Z + ( layerSize * 4 );
	
	// Opacity
	float visibility = 1.0f;
	float d = (shadowCoord.z / shadowCoord.w)*0.5f + 0.5f;
	// float d = shadowDepth;
	vec4 opacity = texture( opacityMap, shadowUV );

	// float dd = texture(headDepthMap, shadowUV).r + 0.0001f;

	if( d > z ) {
		if		( d < layer1Z )	{ visibility -= opacity.r; }
		else if ( d < layer2Z )	{ visibility -= opacity.g; }
		else if ( d < layer3Z )	{ visibility -= opacity.b; }
		else					{ visibility -= opacity.a; }
		// else { visibility = 1; }
	}
	

	// float shadowFactor = 1.;
	// float headZ = texture( depthMap, shadowUV ).r + 0.001f;
	// if( d > headZ ) visibility = 0;
	// if(texture(depthMap, shadowUV).r > shadowDepth)
	//	 shadowFactor = 0;

	// float shadowFactor = PCSS(depthMap, shadowUV, z, lightSize);

	float r_offset = rand()*2*pi;
	float shadowFactor = 1;
	
	for(int k=0; k<64; k++){
		float dd = texture(depthMap, shadowUV + VogelDiskSample(k, 64, r_offset) * 0.001f * lightSize).r + shadowBias;


		if( dd < d ) {
			
			shadowFactor -= 1/64.f;
		}
	}


	vec3 toLight = lightPos - worldPos;
	vec3 Li = lightColor / dot(toLight, toLight) * lightFactor;

	vec3 w_i = normalize( toLight );
	vec3 w_o = normalize( camPos - worldPos );
	vec3 N = normalize( normal );
	Li *= max(dot(N,w_i),0.001);

	vec3 amb = vec3(0.15);
	vec3 diff = vec3(0);
	if(Pass == 0){
		diff = albedo.rgb;// / pi;
	}
	else{
		diff = vec3(1);
	}
	vec3 spec = specularPoint(N, w_i, w_o, albedo.rgb, roughness);
	
	vec3 color = (diff + spec) * Li;

	outColor = vec4( (amb + color) * shadowFactor, albedo.a);
	// outColor = vec4( (amb + color) * visibility, albedo.a);
}