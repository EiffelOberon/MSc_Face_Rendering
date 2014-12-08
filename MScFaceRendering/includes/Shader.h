#ifndef SHADER_H
#define SHADER_H

enum shaderType {VERTEX_SHADER, FRAGMENT_SHADER, GEOMETRY_SHADER,
	TESSELLETION_CONTROL_SHADER, TESSELLATION_EVALUATION_SHADER, COMPUTE_SHADER};

#include <string>
#include <GL/glew.h>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <iostream>


class Shader{
private:
	enum shaderType m_type;
	GLuint m_ID;
	std::string m_name;
	std::string m_source; 
public:

	Shader::Shader();
	Shader::Shader(std::string name, shaderType type);
	Shader::Shader(std::string name, std::string filename, shaderType type);

	void loadFromFile(std::string fileName);
	void loadFromString(std::string source);

	void setSource(std::string source);

	GLuint getID() const;
	enum shaderType getType() const;
	std::string getName() const;
	std::string getSource() const;
	bool compile();


	Shader::~Shader();

};

#endif

