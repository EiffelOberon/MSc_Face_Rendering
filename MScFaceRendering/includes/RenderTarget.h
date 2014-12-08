#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H



/*
 * Refactoring Note: This has been added from the cifRenderer project. For this reason it may not be fully integrated yet or has more features than needed. Probably gonna prune it a bit soon.
*/



#include <string>
#include <GL/glew.h>
#include <vector>
#include <stdlib.h>
#include <iostream>
#include "glm/glm.hpp"

class RenderTarget{

private:

	// Dimensions 
	unsigned int m_width;
	unsigned int m_height; 

	// IDs
	GLuint m_FBO;
	std::vector<GLuint> m_colourBuffers;
	GLuint m_depthBuffer; 



	void updateDrawBuffers();

public:

	RenderTarget();
	RenderTarget(unsigned int width, unsigned int height);
	RenderTarget(unsigned int width, unsigned int height, GLint depthFormat, bool linearFiltering);
	RenderTarget(unsigned int width, unsigned int height, GLint internalFormat, GLenum format, bool HDR);
	RenderTarget(unsigned int width, unsigned int height, GLint depthFormat, GLint internalFormat, GLenum format, bool HDR);
	RenderTarget(unsigned int width, unsigned int height, GLint internalFormat, GLenum format, bool HDR, bool linearFiltering,  bool mipmap, bool aniso);
	RenderTarget(unsigned int width, unsigned int height, GLint internalFormat, GLenum format, GLint magFilter, GLint minFilter, bool HDR, bool aniso);

	~RenderTarget();

	// Add buffers
	void addBuffer(GLint internalFormat, GLenum format);
	void addBuffers(unsigned int number, GLint internalFormat, GLenum format);
	void addBufferHDR(GLint internalFormat, GLenum format);
	void addBuffersHDR(unsigned int number, GLint internalFormat, GLenum format);


	void setDepthBuffer(GLint depthFormat);

	// Getters
	GLuint getFBO() const;
	unsigned int getNumberOfColBuffers() const;
	GLuint getBuffer(unsigned int index) const;
	GLuint getDepthBuffer() const;
	glm::ivec2 getSize() const;

	// Various binds
	void bind() const;
	void bindToRead() const;
	void bindToWrite() const;
	void bindBuffer(unsigned int index) const;
	void bindDepthBuffer() const;

	// Misc.
	void setReadBuffer(unsigned int index) const;
	bool isHDR();	// Unused. Unset. 

};



#endif