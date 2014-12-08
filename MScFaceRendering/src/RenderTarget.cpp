#include "RenderTarget.h"


// Constructors
// Notes: revisit the constructor so to reduce code duplication. 

RenderTarget::RenderTarget(){
}

RenderTarget::RenderTarget(unsigned int width, unsigned int height){
	m_height = height; 
	m_width = width;
	glGenFramebuffers(1, &m_FBO);
}


RenderTarget::RenderTarget(unsigned int width, unsigned int height, GLint internalFormat, GLenum format, bool HDR){

	m_height = height; 
	m_width = width;

	GLuint texID;

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glGenTextures(1, &texID);

	glBindTexture(GL_TEXTURE_2D, texID);

	if(HDR)
		glTexImage2D(GL_TEXTURE_2D,0,internalFormat, m_width, m_height,0, format, GL_FLOAT, 0);
	else 
		glTexImage2D(GL_TEXTURE_2D,0,internalFormat, m_width, m_height,0, format, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texID,0);
	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};

	glDrawBuffers(1,drawBuffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE){
		printf("Error in framebuffer generation...\n");
		exit(EXIT_FAILURE);
	}


	m_colourBuffers.push_back(texID);

}

RenderTarget::RenderTarget(unsigned int width, unsigned int height, GLint depthFormat, GLint internalFormat, GLenum format, bool HDR){

	m_height = height; 
	m_width = width;

	GLuint texID;

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	if(HDR)
		glTexImage2D(GL_TEXTURE_2D,0,internalFormat, m_width, m_height,0, format, GL_FLOAT, 0);
	else 
		glTexImage2D(GL_TEXTURE_2D,0,internalFormat, m_width, m_height,0, format, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texID,0);
	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1,drawBuffers);

	glGenTextures(1,&m_depthBuffer);
	glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, depthFormat, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthBuffer, 0);


	if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE){
		printf("Error in framebuffer generation...\n");
		exit(EXIT_FAILURE);
	}

	m_colourBuffers.push_back(texID);


}




RenderTarget::RenderTarget(unsigned int width, unsigned int height, GLint internalFormat, GLenum format, bool HDR, bool linearFiltering,   bool mipmap, bool aniso){


	m_height = height; 
	m_width = width;

	GLuint texID;

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glGenTextures(1, &texID);

	glBindTexture(GL_TEXTURE_2D, texID);

	if(HDR)
		glTexImage2D(GL_TEXTURE_2D,0,internalFormat, m_width, m_height,0, format, GL_FLOAT, 0);
	else 
		glTexImage2D(GL_TEXTURE_2D,0,internalFormat, m_width, m_height,0, format, GL_UNSIGNED_BYTE, 0);


	if(linearFiltering){
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if(mipmap)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);	
		else
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}else{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		if(mipmap)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST); // I whish I had more flexibility... solve that constructor ambiguity... Important...	
		else
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
	if(mipmap)
		glGenerateMipmap(GL_TEXTURE_2D);
	if(aniso){
		GLfloat max_aniso;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
	}

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texID,0);
	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1,drawBuffers);



	if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE){
		printf("Error in framebuffer generation...\n");
		exit(EXIT_FAILURE);
	}


	m_colourBuffers.push_back(texID);

}


RenderTarget::RenderTarget(unsigned int width, unsigned int height, GLint depthFormat, bool linearFiltering){
	m_height = height; 
	m_width = width;

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);


	glGenTextures(1,&m_depthBuffer);
	glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, depthFormat, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	if(linearFiltering){
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 

	}else{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	}

	float clampColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthBuffer, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE){
		printf("Error in framebuffer generation...\n");
		exit(EXIT_FAILURE);
	}

}



RenderTarget::RenderTarget(unsigned int width, unsigned int height, GLint internalFormat, GLenum format, GLint magFilter, GLint minFilter, bool HDR, bool aniso){


	m_height = height; 
	m_width = width;

	GLuint texID;

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glGenTextures(1, &texID);

	glBindTexture(GL_TEXTURE_2D, texID);

	if(HDR)
		glTexImage2D(GL_TEXTURE_2D,0,internalFormat, m_width, m_height,0, format, GL_FLOAT, 0);
	else 
		glTexImage2D(GL_TEXTURE_2D,0,internalFormat, m_width, m_height,0, format, GL_UNSIGNED_BYTE, 0);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
	if(minFilter== GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST || minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST)
		glGenerateMipmap(GL_TEXTURE_2D);
	if(aniso){
		GLfloat max_aniso;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
	}

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texID,0);
	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1,drawBuffers);



	if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE){
		printf("Error in framebuffer generation...\n");
		exit(EXIT_FAILURE);
	}


	m_colourBuffers.push_back(texID);

}



RenderTarget::~RenderTarget(){

	// TOFIX around the rest of the project. Called too often. 

	//glDeleteFramebuffers(1, &m_FBO);
	//for(GLuint tex : m_colourBuffers){
	//	glDeleteTextures(1, &tex);
	//}
	//glDeleteTextures(1, &m_depthBuffer);
}


// Buffer adding
void RenderTarget::updateDrawBuffers(){

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	std::vector<GLenum> drawBuffers;
	for(unsigned int i=0; i<m_colourBuffers.size(); i++){
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, m_colourBuffers.at(i), 0);
		drawBuffers.push_back(GL_COLOR_ATTACHMENT0+i);
	}
	glDrawBuffers(drawBuffers.size(), &drawBuffers[0]);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE){
		printf("Error in framebuffer generation...\n");
		exit(EXIT_FAILURE);
	}

}


void RenderTarget::addBuffer(GLint internalFormat, GLenum format){

	GLuint texID;

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D,0,internalFormat, m_width, m_height,0, format, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

	m_colourBuffers.push_back(texID);
	updateDrawBuffers();

}

void RenderTarget::addBuffers(unsigned int number, GLint internalFormat, GLenum format){

	for (unsigned int i=0; i<number; i++){
		GLuint texID;

		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexImage2D(GL_TEXTURE_2D,0,internalFormat, m_width, m_height,0, format, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

		m_colourBuffers.push_back(texID);
	}
	updateDrawBuffers();
}

void RenderTarget::addBufferHDR(GLint internalFormat, GLenum format){
	GLuint texID;

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D,0,internalFormat, m_width, m_height,0, format, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

	m_colourBuffers.push_back(texID);
	updateDrawBuffers();

}

void RenderTarget::addBuffersHDR(unsigned int number, GLint internalFormat, GLenum format){

	for (unsigned int i=0; i<number; i++){
		GLuint texID;

		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexImage2D(GL_TEXTURE_2D,0,internalFormat, m_width, m_height,0, format, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

		m_colourBuffers.push_back(texID);
	}
	updateDrawBuffers();

}

void RenderTarget::setDepthBuffer(GLint depthFormat){

	glGenTextures(1,&m_depthBuffer);
	glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, depthFormat, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthBuffer, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE){
		printf("Error in framebuffer generation...\n");
		exit(EXIT_FAILURE);
	}

}


// Getters 
GLuint RenderTarget::getFBO() const{
	return m_FBO;
}

unsigned int RenderTarget::getNumberOfColBuffers() const{
	return m_colourBuffers.size();
}

GLuint RenderTarget::getBuffer(unsigned int index) const{
	if(index > m_colourBuffers.size()){
		std::cout<<"Index bigger than number of textures. Returning first attachment. \n";
		return m_colourBuffers.at(0);
	}

	return m_colourBuffers.at(index);
}

GLuint RenderTarget::getDepthBuffer() const{
	return m_depthBuffer;
}

glm::ivec2 RenderTarget::getSize() const{
	return glm::ivec2(m_width, m_height);
}

// Various bindings

void RenderTarget::bind() const{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
}

void RenderTarget::bindToRead() const{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
}

void RenderTarget::bindToWrite() const{ 
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
}

void RenderTarget::bindBuffer(unsigned int index) const{
	if(index > m_colourBuffers.size()){
		std::cout<<"Index bigger than number of textures. Binding first attachment. \n";
		glBindTexture(GL_TEXTURE_2D, m_colourBuffers.at(0));
	}else
		glBindTexture(GL_TEXTURE_2D, m_colourBuffers.at(index));
}

void RenderTarget::bindDepthBuffer() const{
	glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
}

void RenderTarget::setReadBuffer(unsigned int index) const{
	if(index > m_colourBuffers.size()){
		std::cout<<"Index bigger than number of textures. Setting first attachment. \n";
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}else
		glReadBuffer(GL_COLOR_ATTACHMENT0+index);

}

