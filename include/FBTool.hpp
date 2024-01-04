#ifndef FBTool_hpp
#define FBTool_hpp

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include "ShaderTool.hpp"
#include "ModelTool.hpp"

#include <stdio.h>
#include <iostream>

struct _PREV_STATE_ {
	GLint  fbo;
	GLint  viewport[4];
	GLint  cullMode = GL_BACK;
	GLint  cullFace = GL_FALSE;
	GLint  depthTest = GL_FALSE;
	void capture() {
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
		glGetIntegerv(GL_VIEWPORT, viewport);
		glGetIntegerv(GL_CULL_FACE_MODE, &cullMode);

		cullFace = glIsEnabled(GL_CULL_FACE);
		depthTest = glIsEnabled(GL_DEPTH_TEST);
	}
	void restore() {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
		glCullFace(cullMode);

		if (cullFace)	glEnable(GL_CULL_FACE);
		else			glDisable(GL_CULL_FACE);
		if (depthTest)	glEnable(GL_DEPTH_TEST);
		else			glDisable(GL_DEPTH_TEST);
	}
};

struct Framebuffer {
protected:
	size_t w = 0;
	size_t h = 0;
	GLuint fbo = 0;
	GLuint color = 0;
	GLuint depth = 0;
	
	GLint oldFB = 0;
	GLint oldVP[4];

	Shader fbShader;

	_PREV_STATE_ prevState;

	float near = 1.0f;
	float far = 100.0f;

public:
	inline GLuint getColor( void ) const {
		return color;
	}
	inline GLuint getDepth( void ) const {
		return depth;
	}
	static void setTexParam(GLuint minFilter = GL_LINEAR, GLuint warp = GL_CLAMP_TO_EDGE) {
		float maxAniso;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, warp);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, warp);
	}
	void storeFramebufferState() {
		prevState.capture();
	}
	void restoreFramebufferState() {
		prevState.restore();
	}
	void create( int ww, int hh, bool withDepthBuffer = false, GLenum type = GL_UNSIGNED_BYTE,
			GLenum internal = GL_RGBA32F, GLenum format = GL_RGBA) {
		if (ww == w && hh == h) return;

		w = ww;
		h = hh;
		printf("FB creation: %zd x %zd\n", w, h);
		clearGL();

		glGenTextures(1, &color);
		glBindTexture(GL_TEXTURE_2D, color);
		glTexImage2D(GL_TEXTURE_2D, 0, internal, ww, hh, 0, format, type, 0);
		setTexParam();

		if ( withDepthBuffer ) {
			glGenTextures(1, &depth);
			glBindTexture(GL_TEXTURE_2D, depth);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, ww, hh, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			setTexParam();
		}
		
		glGenFramebuffers(1, &fbo);
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFB);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cerr << "FBO is incomplete";

		glBindFramebuffer(GL_FRAMEBUFFER, oldFB);

	}
	void use() {
		storeFramebufferState();
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	}
	void unuse() {
		restoreFramebufferState();
	}
	void clearGL( void ) {
		if (color) glDeleteTextures(1, &color); color = 0;
		if (depth) glDeleteTextures(1, &depth); depth = 0;
		if (fbo) glDeleteFramebuffers(1, &fbo); fbo = 0;
	}
	void bindTex(int slot, bool isDepth = false) {
		glActiveTexture(GL_TEXTURE0 + slot);
		if(isDepth)
			glBindTexture(GL_TEXTURE_2D, depth);
		else
			glBindTexture(GL_TEXTURE_2D, color);
	}
	void bindTex(int slot, const Shader& shader, const std::string& name, bool isDepth = false) {
		bindTex(slot, isDepth);
		shader.setInt(name, slot);
	}

};


#endif /* FBTool_hpp */