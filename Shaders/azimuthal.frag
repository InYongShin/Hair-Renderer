#version 410 core

const float PI = 3.141592f;

out vec4 outColor;
in vec2 texCoord;

uniform sampler2D longitudinalTex;
uniform float eta;
uniform vec3 absorption;

uniform int Pass = 0;


float Phi( int p, float gammaI, float gammaT ) {
	return 2*p*gammaT - 2*gammaI + p*PI;
}

float gammaI( float h ) {
	return asin(h);
}

float gammaT( float h, float eta1 ) {
	return asin(h/eta1);
}

float Snell1( float eta, float gamma ){
	float a = sqrt(eta*eta - sin(gamma)*sin(gamma));
	float b = cos(gamma);
	return a / b;
}

float Snell2( float eta, float gamma ){
	float eta2 = eta*eta;
	float a = eta2 * cos(gamma);
	float b = sqrt(eta2 - sin(gamma) * sin(gamma));
	return a / b;

	// float a = sqrt(eta*eta - sin(gamma)*sin(gamma));
	// float b = cos(gamma);
	// return a / b;
}

float Fresnel( float n1, float n2, float gamma ) {
	float F0 = pow( abs((n1-n2))/(n1+n2), 2);
	return F0 + (1-F0) * (pow(1.-cos(gamma), 5) );
}

vec3 absorb( vec3 absorption, float gammaT ) {
	return exp(-2*absorption*(1+cos(2*gammaT)));
}

vec3 attenuate( vec2 etaP, float h ){
	float _gammaI = gammaI( h );
	float _gammaT = gammaT( h, etaP.x );
	float F_i = Fresnel(etaP.x, etaP.y, _gammaI);
	return vec3(F_i);
}

vec3 attenuate( int p, vec2 etaP, float h, vec3 absorption, float thetaD ){
	float _gammaI = gammaI( h );
	float _gammaT = gammaT( h, etaP.x );
	
	// float F_i = Fresnel(etaP.x, etaP.y, _gammaI);
	
	float F_i = Fresnel(etaP.x, 1, _gammaI);
	float F_t = Fresnel(1./etaP.x, 1, _gammaT);

	return pow(1-F_i, 2) * pow(F_t, p-1) * pow(absorb(absorption, h), vec3(p));

	// float F_t = Fresnel(1/etaP.x, 1/etaP.y, _gammaT);
	// vec3 absorptionP = absorption/cos(thetaD);
	// return vec3(F_t);
	// return pow(1-F_i, 2) * pow(F_t, p-1) * pow(absorb(absorptionP, _gammaT), vec3(p));
	// return pow(1-F_i, 2) * pow(F_t, p-1) * pow(absorb(absorptionP, _gammaT), vec3(p));
}

float dphi( float eta1, float h ) {
	float a = 2 / (eta1 * sqrt(1-(pow(h,2)/pow(eta1,2) ) ) );
	float b = 2 / sqrt(1-h*h);
	return a - b;
}

vec3 NR( vec2 etaP, float h ) {
	vec3 a = attenuate(etaP, h);
	float _dphi = dphi(etaP.x, h);
	return a * abs(1./(2.*_dphi));
	// return a;// * abs(1./(2.*_dphi));
}

vec3 NTT( vec2 etaP, float h, vec3 absorption, float thetaD ) {
	vec3 a = attenuate(1, etaP, h, absorption, thetaD);
	float _dphi = dphi(etaP.x, h);
	return a * abs(1./(2.*_dphi));
	// return a;// * abs(1./(2.*_dphi));
}

vec3 NTRT( vec2 etaP, float h, vec3 absorption, float thetaD ) {
	vec3 a = attenuate(2, etaP, h, absorption, thetaD);
	float _dphi = dphi(etaP.x, h);
	return a * abs(1./(2.*_dphi));
	// return a;// * abs(1./(2.*_dphi));
}


void main( void ) {
	float phiI = texCoord.x*2*PI - PI;
	float phiR = texCoord.y*2*PI - PI;
	// float phiI = texCoord.x*PI - PI/2;
	// float phiR = texCoord.y*PI - PI/2;

	vec2 uv = vec2(1-texCoord.x, texCoord.y);
	// float eta1 = texture(longitudinalTex, uv).a;// + eta;
	float thetaD = texture(longitudinalTex, uv).a;
	vec2 etaP = vec2(Snell1(eta, thetaD), Snell2(eta, thetaD));

	float phiD = phiI - phiR;
	float gammaI = -0.5 * phiD;
	float h = sin(gammaI);

	// outColor = vec4(texture(longitudinalTex, uv).rgb, 1);

	vec3 N = vec3(0);
	if( Pass == 0 ) {
		N = NR(etaP, h);
	}
	else if ( Pass == 1 ) {
		N = NTT(etaP, h, absorption, thetaD);
	}
	else if ( Pass == 2 ) {
		N = NTRT(etaP, h, absorption, thetaD);
	}
	else {
		N = vec3(0);
	}
	outColor = vec4(N, 1);
}