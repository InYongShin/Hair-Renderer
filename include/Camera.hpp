#ifndef Camera_hpp
#define Camera_hpp

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include "ShaderTool.hpp"

const float RADIANPI = 3.141592/180.;


struct Camera {
	float distance = 8.0f;
	float yaw = 0.0f;
	float pitch = 0.0f;
	float fov = 60.0f;
	float zNear = 0.1f;
	float zFar = 100.f;
	int width;
	int height;
	glm::vec3 sceneCenter = { 0, 1.7f, 0 };

	inline glm::vec3 getPos() const {
		return glm::vec3(glm::rotate(yaw, glm::vec3(0, 1, 0)) 
			* rotate(pitch, glm::vec3(-1, 0, 0)) * glm::vec4(0, 1.7f, distance, 1));
	}
	inline glm::mat4 getViewMat() const {
		return glm::lookAt(getPos(), sceneCenter, glm::vec3(0, 1, 0));
	}
	inline glm::mat4 getProjMat() const {
		return glm::perspective(fov * RADIANPI, width / float(height), zNear, zFar);
	}
	inline float getFov() const {
		return fov * RADIANPI;
	}
	inline void setFrameSize(int w, int h) {
		width = w; height = h;
	}
	inline void setToProgram(Shader& shader) const {
		shader.setMat4("viewMat", getViewMat());
		shader.setMat4("projMat", getProjMat());
	}
	inline void setPosToProgram(Shader& shader) const {
		shader.setVec3("camPos", getPos());
	}
	void mouseMoved(float dx, float dy) {
		yaw -= ( dx / 300.f );
		pitch += ( dy / 300.f );
		pitch = glm::clamp(pitch, -1.3f, 1.3f);
	}
	void scrolled(float dx, float dy) {
		// Fov : zoom
		// fov += dy/30.0f;

		// cameraDistance : dolly
		distance *= pow(1.001, -dy); 
	}

};

#endif /* Camera_hpp */