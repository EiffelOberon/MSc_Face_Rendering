#include "../includes/Texture.h"


Texture::Texture(void){ }

Texture::Texture(const std::string &filename){

	m_isHDR = false;
	loadSTBI(filename);

	GLenum format;

	switch (m_imageChannels){
	case 1: 
		format = GL_RED;
	case 2:
		format = GL_RG;
	case 3:
		format = GL_RGB;
	case 4:
		format = GL_RGBA;
	default:
		format = GL_RGB;
		break;
	}


	if(m_isHDR){
		if(format == GL_RED){
			initTexture(GL_R16F, format, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_FLOAT);
		}if(format == GL_RG){
			initTexture(GL_RG16F, format, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_FLOAT);
		}if(format == GL_RGB){
			initTexture(GL_RGB16F, format, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_FLOAT);
		}else if(format == GL_RGBA){
			initTexture(GL_RGBA16F, format, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_FLOAT);
		}
	}else{
		initTexture(format,format,GL_LINEAR,GL_LINEAR_MIPMAP_LINEAR,GL_UNSIGNED_BYTE);
	}


	m_imageKept = true;
}

Texture::Texture(const std::string &filename, GLenum magFilter, GLenum minFilter, GLenum type){
	loadSTBI(filename);

	GLenum format;

	switch (m_imageChannels){
	case 1: 
		format = GL_RED;
	case 2:
		format = GL_RG;
	case 3:
		format = GL_RGB;
	case 4:
		format = GL_RGBA;
	default:
		break;
	}

	if(format == GL_RED){
		initTexture(GL_R16F, format, magFilter, minFilter, GL_FLOAT);
	}if(format == GL_RG){
		initTexture(GL_RG16F, format,  magFilter, minFilter, GL_FLOAT);
	}if(format == GL_RGB){
		initTexture(GL_RGB16F, format,  magFilter, minFilter, GL_FLOAT);
	}else if(format == GL_RGBA){
		initTexture(GL_RGBA16F, format,  magFilter, minFilter, GL_FLOAT);
	}

	m_imageKept = true;

}

Texture::Texture(const std::string &filename, GLuint internalFormat, GLenum format, GLenum magFilter, GLenum minFilter, GLenum type){
	loadSTBI(filename);
	initTexture(internalFormat,format,magFilter,minFilter,type);
	m_imageKept = true;

}




Texture::~Texture(void){

	glDeleteTextures(1, &m_ID);
	if(m_imageKept)
		free(m_image);
}

void Texture::loadSTBI(const std::string &filename){

	if(filename.substr( filename.length() - 3 ) == "hdr ") m_isHDR = true;

	FILE* file = fopen(filename.c_str(),"rb");
	if(!file){
		// Use a better error handling
		std::cout<<"Texture in"<< filename <<" couldn't be opened, an error is expected to happen.\n";
	}
	m_image = stbi_load_from_file(file,&m_width, &m_height, &m_imageChannels, 0);


	fclose(file);


}

void Texture::initTexture(GLuint internalFormat, GLenum format, GLuint magFilter, GLuint minFilter, GLuint type){

	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_2D, m_ID);
	glTexImage2D(GL_TEXTURE_2D,0,internalFormat,m_width,m_height, 0, format, type, m_image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter); 
	if(minFilter== GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST || minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST)
		glGenerateMipmap(GL_TEXTURE_2D);

}

GLuint Texture::getID() const{
	return m_ID;
}

int Texture::getWidth() const{
	return m_width;
}

int Texture::getHeight() const{
	return m_height;
}

bool Texture::imageIsKept() const{
	return m_imageKept;
}


void Texture::deleteImageData(){
	free(m_image);
	m_imageKept = false;
}


void Texture::bind() const{
	glBindTexture(GL_TEXTURE_2D, m_ID);
}

bool Texture::isHDR() const{
	return m_isHDR;
}

