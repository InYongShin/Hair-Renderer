#version 410 core

const float pi = 3.141592f;

out vec4 outColor;

uniform float alphaR;
uniform float betaR;

uniform float eta;

in vec2 texCoord;

float gauss( float sigma, float x ) {
	if(sigma == 0) return 1.;
	return 1./(sigma*sqrt(2.*pi)) * exp((-.5)*((x*x)/(sigma*sigma))); 
}

void main( void ) {
	float thetaI = texCoord.x * pi - pi*.5;
	float thetaR = texCoord.y * pi - pi*.5;

	float thetaH = (thetaI + thetaR) * .5;
	float thetaD = (thetaR - thetaI) * .5;

	float alphaTT = -alphaR*.5;
	float alphaTRT = -3.*alphaR/2.;
	float betaTT = betaR*.5;
	float betaTRT = 2.*betaR;

	float MR = gauss(betaR, thetaH-alphaR) / pow(cos(thetaD),2);
	float MTT = gauss(betaTT, thetaH-alphaTT) / pow(cos(thetaD),2);
	float MTRT = gauss(betaTRT, thetaH-alphaTRT) / pow(cos(thetaD),2);
	// float eta1 = Snell1(eta, thetaD);

	outColor = vec4(MR, MTT, MTRT, thetaD);
}