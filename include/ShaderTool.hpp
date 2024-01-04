#ifndef ShaderTool_hpp
#define ShaderTool_hpp

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

inline std::string loadText(const std::string& path) {
	if (path.empty()) {
		std::cout << "path is empty" << std::endl;
		// TODO : assert(false);
		exit(1);
	}

	std::string code;
	std::ifstream file;

	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		file.open(path);
		std::stringstream shaderStream;
		shaderStream << file.rdbuf();
		file.close();
		code = shaderStream.str();
	}
	catch (std::ifstream::failure& e) {
		std::cout << "Shader file not succesfully read." << std::endl;
		std::cout << e.code() << std::endl;
	}

	return code;
}


struct Shader {
public:
	GLuint programID = 0;
	GLuint vertexID = 0;
	GLuint fragmentID = 0;
	GLuint geometryID = 0;
	GLuint tessControlID = 0;
	GLuint tessEvaluateID = 0;

	inline void loadShaders(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr, 
		const char* tessControlPath = nullptr, const char* tessEvaluatePath = nullptr) {
		
		programID = glCreateProgram();

		// Vertex Shader
		vertexID = glCreateShader(GL_VERTEX_SHADER);
		std::string vertexCode = loadText(vertexPath);
		const char* vCode = vertexCode.c_str();
		glShaderSource(vertexID, 1, &vCode, NULL);
		glCompileShader(vertexID);
		checkCompileErrors(vertexID, "VERTEX");
		glAttachShader(programID, vertexID);

		// Tessellation Control Shader
		if (tessControlPath != nullptr) {
			tessControlID = glCreateShader(GL_TESS_CONTROL_SHADER);
			std::string tessControlCode = loadText(tessControlPath);
			const char* tcCode = tessControlCode.c_str();
			glShaderSource(tessControlID, 1, &tcCode, NULL);
			glCompileShader(tessControlID);
			checkCompileErrors(tessControlID, "TESSCONTROL");
			glAttachShader(programID, tessControlID);
		}

		// Tessellation Evaluate Shader
		if (tessEvaluatePath != nullptr) {
			tessEvaluateID = glCreateShader(GL_TESS_EVALUATION_SHADER);
			std::string tessEvaluateCode = loadText(tessEvaluatePath);
			const char* teCode = tessEvaluateCode.c_str();
			glShaderSource(tessEvaluateID, 1, &teCode, NULL);
			glCompileShader(tessEvaluateID);
			checkCompileErrors(tessEvaluateID, "TESSEVALUATE");
			glAttachShader(programID, tessEvaluateID);
		}

		// Geometry Shader
		if (geometryPath != nullptr) {
			geometryID = glCreateShader(GL_GEOMETRY_SHADER);
			std::string geometryCode = loadText(geometryPath);
			const char* gCode = geometryCode.c_str();
			glShaderSource(geometryID, 1, &gCode, NULL);
			glCompileShader(geometryID);
			checkCompileErrors(geometryID, "GEOMETRY");
			glAttachShader(programID, geometryID);
		}

		// Fragment Shader
		fragmentID = glCreateShader(GL_FRAGMENT_SHADER);
		std::string fragmentCode = loadText(fragmentPath);
		const char* fCode = fragmentCode.c_str();
		glShaderSource(fragmentID, 1, &fCode, NULL);
		glCompileShader(fragmentID);
		checkCompileErrors(fragmentID, "FRAG");
		glAttachShader(programID, fragmentID);


		glLinkProgram(programID);
		glUseProgram(programID);
		checkCompileErrors(programID, "PROGRAM");
	}
	inline void use() {
		glUseProgram(programID);
	}
	inline void unuse() {
		
		glUseProgram(programID);
	}
	inline void setBool(const std::string& name, bool value) const {
		glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)value);
	}
	inline void setInt(const std::string& name, int value) const {
		glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
	}
	inline void setFloat(const std::string& name, float value) const {
		glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
	}
	inline void setVec2(const std::string& name, const glm::vec2& value) const {
		glUniform2fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
	}
	inline void setVec2(const std::string& name, float x, float y) const {
		glUniform2f(glGetUniformLocation(programID, name.c_str()), x, y);
	}
	inline void setVec3(const std::string& name, const glm::vec3& value) const {
		glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
	}
	inline void setVec3(const std::string& name, float x, float y, float z) const {
		glUniform3f(glGetUniformLocation(programID, name.c_str()), x, y, z);
	}
	inline void setVec4(const std::string& name, const glm::vec4& value) const {
		glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
	}
	inline void setVec4(const std::string& name, float x, float y, float z, float w) {
		glUniform4f(glGetUniformLocation(programID, name.c_str()), x, y, z, w);
	}
	inline void setMat2(const std::string& name, const glm::mat2& mat) const {
		glUniformMatrix2fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	} 
	inline void setMat3(const std::string& name, const glm::mat3& mat) const {
		glUniformMatrix3fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	inline void setMat4(const std::string& name, const glm::mat4& mat) const {
		glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	void cleanUp() {
		if (programID)
			glDeleteProgram(programID);
		if (vertexID)
			glDeleteShader(vertexID);
		if (fragmentID)
			glDeleteShader(fragmentID);
		if (geometryID)
			glDeleteShader(geometryID);
		if (tessControlID)
			glDeleteShader(tessControlID);
		if (tessEvaluateID)
			glDeleteShader(tessEvaluateID);
		programID = vertexID = fragmentID = geometryID = tessControlID = tessEvaluateID = 0;
	}
	~Shader() {
		cleanUp();
	}

private:
	// source: learnopengl
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
	void checkCompileErrors(GLuint shader, std::string type)
	{
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
};

#endif /* ShaderTool.hpp */