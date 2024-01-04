#ifndef Light_hpp
#define Light_hpp

#include <glm/glm.hpp>
#include "ShaderTool.hpp"

struct Light {
	glm::vec3 position;
	glm::vec3 color = glm::vec3(1);
	float lightSize = 1.;
	float lightFactor = 6.;

	inline glm::vec3 getPos() const {
		return position;
	}
	inline void setPos(glm::vec3 pos) {
		position = pos;
	}
	inline glm::vec3 getColor() const {
		// return color * lightFactor;
		return color;
	}
	inline void setColor(glm::vec3 color) {
		this->color = color;
	}
	inline void setPosToProgram(Shader& shader) const {
		shader.setVec3("lightPos", position);
	}
	inline void setFactorToProgram(Shader& shader) const {
		shader.setFloat("lightFactor", lightFactor);
	}

	Light() {}
	~Light() {}
};

#endif /* Light_hpp */
