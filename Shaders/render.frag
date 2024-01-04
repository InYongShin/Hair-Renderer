
#version 410 core

const float pi = 3.141592f;
const float GOLDEN_ANGLE = 2.39996;

out vec4 outColor;

in vec4 shadowCoord;
in float shadowDepth;
in vec3 worldPos;
in vec3 tangent;

uniform vec4 albedo;

uniform vec3 camPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightFactor;

uniform float layerSize;

uniform float lightSize;

uniform float shadowBias;

uniform sampler2D longitudinalTex;
uniform sampler2D azimuthalRTex;
uniform sampler2D azimuthalTTTex;
uniform sampler2D azimuthalTRTTex;

uniform sampler2D depthMap;
uniform sampler2D opacityMap;
// uniform sampler2D headDepthMap;

uniform int Pass = 0;


float tonemap_sRGB(float u) {
	float u_ = abs(u);
	return  u_>0.0031308?( sign(u)*1.055*pow( u_,0.41667)-0.055):(12.92*u);
}

vec3 tonemap( vec3 rgb, mat3 csc, float gamma ){
	vec3 rgb_ = csc*rgb;
	if( abs( gamma-2.4) <0.01 ) // sRGB
		return vec3( tonemap_sRGB(rgb_.r), tonemap_sRGB(rgb_.g), tonemap_sRGB(rgb_.b) );
	return sign(rgb_)*pow( abs(rgb_), vec3(1./gamma) );
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


void main( void ) {

	// Depth
	vec2 shadowUV = (shadowCoord.xy / shadowCoord.w)*0.5f + 0.5f;
	float z = texture(depthMap, shadowUV).r + shadowBias;
	float layer1Z = z		+   layerSize;
	float layer2Z = layer1Z + ( layerSize * 2 );
	float layer3Z = layer2Z + ( layerSize * 3 );
	float layer4Z = layer3Z + ( layerSize * 4 );
	float visibility = 1.0f;

	// Opacity
	float d = (shadowCoord.z / shadowCoord.w)*0.5f + 0.5f;
	// float d = shadowDepth;

	
	vec4 opacity = texture( opacityMap, shadowUV );
	// if		( d > z + 0.001f )		{ visibility = 0; }
	if( d > z ) {
		if		( d < layer1Z )	{ visibility -= opacity.r; }
		else if ( d < layer2Z )	{ visibility -= opacity.g; }
		else if ( d < layer3Z )	{ visibility -= opacity.b; }
		else					{ visibility -= opacity.a; }
	}

	// float shadowFactor = 1.;
	// float headZ = texture( headDepthMap, shadowUV ).r;
	// if( d > headZ - 0.0001f ) visibility = 0;

	float r_offset = rand()*2*pi;
	float shadowFactor = 1.;
	
	for(int k=0; k<64; k++){
		if( texture(depthMap, shadowUV + VogelDiskSample(k, 64, r_offset) * 0.001f * lightSize).r + shadowBias < d ) {
			shadowFactor -= 1/64.f;
		}
	}
	shadowFactor = max(shadowFactor, 0.3);

	vec3 toLight = lightPos - worldPos;
	vec3 Li = lightColor / dot(toLight, toLight) * lightFactor;
	vec3 toCam = normalize(camPos - worldPos);
	vec3 tangentVec = normalize(tangent);
	toLight = normalize(toLight);
	
	float uDotwi = dot(tangentVec, toLight);
	float uDotwr = dot(tangentVec, toCam);
	vec2 theta = vec2( -asin( uDotwi ), asin( uDotwr ) );
	vec2 phi = vec2( atan(toLight.y, length(toLight.xz)), atan(toCam.y, length(toCam.xz)) );

	vec3 spec = vec3(0);
	vec2 Muv = vec2(theta.x/pi+.5, theta.y/pi+.5);
	vec3 M = texture(longitudinalTex, Muv).rgb;

	// outColor = vec4(vec3(Muv.x), 1);
	// return;
	
	// vec2 Nuv = vec2((phi.x+pi)/(2*pi), (phi.y+pi)/(2*pi));
	vec2 Nuv = vec2((phi.x+pi)/(2*pi), (phi.y+pi)/(2*pi));
	vec3 NR = texture(azimuthalRTex, Nuv).rgb;
	vec3 NTT = texture(azimuthalTTTex, Nuv).rgb;
	vec3 NTRT = texture(azimuthalTRTTex, Nuv).rgb;

	spec += ((NR * M.x ) + (NTT * M.y ) + (NTRT * M.z ));
	// spec += (( M.x ) + ( M.y ) + ( M.z ));
	// Li *= max(dot(tangentVec,toLight),0.001);
	spec *= Li;
	
	vec3 diff = albedo.rgb; /// PI;
	
	float alpha = albedo.a;

	 outColor = vec4((spec + diff) /** visibility*/ * shadowFactor, alpha);
	 // outColor = vec4((spec + diff) /** visibility * shadowFactor*/, alpha);
	 if(Pass != 0) outColor = vec4(vec3(0.9, 0.9, 1) * visibility /** shadowFactor*/ , alpha);
	
	// outColor = vec4(spec + diff, alpha);
}