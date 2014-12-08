#ifndef  TEXTURE_H
#define  TEXTURE_H


/*
* Refactoring Note: This has been added from the cifRenderer project. For this reason it may not be fully integrated yet or has more features than needed. Probably gonna prune it a bit soon.
*					The original project used EXR for HDR image instead of .hdr, this will be added as option soon.  
*
*/


#include <string>
#include <GL/glew.h>
#include <stdlib.h>
#include <iostream>
#include "glm/glm.hpp"
#include "stbi_image.h"


class Texture{
private:

	GLuint m_ID;
	unsigned char* m_image; // Atm, the image is NOT kept in memory. May change it soon. 

	int m_width;
	int m_height;
	int m_imageChannels;

	bool m_imageKept;
	bool m_isHDR;


	void loadSTBI(const std::string &filename);  // Atm everything is loaded with STBI, planning to use other loader
	void initTexture(GLuint internalFormat, GLenum format, GLuint magFilter, GLuint minFilter, GLenum type);


public:
	Texture(void);
	Texture(const std::string &filename);
	Texture(const std::string &filename, GLenum magFilter, GLenum minFilter, GLenum type);
	Texture(const std::string &filename, GLuint internalFormat, GLenum format, GLenum magFilter, GLenum minFilter, GLenum type);


	~Texture(void);

	GLuint getID() const;
	int getWidth() const;  
	int getHeight() const;

	bool isHDR() const;
	bool imageIsKept() const;

	void deleteImageData();
	void bind() const;


};

#endif // ! TEXTURE_H
