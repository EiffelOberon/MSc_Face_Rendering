#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H


#define VERBOSE 0

#define LIGHT_UBO_BINDING_POINT 0



// Here vertex attributes naming conventions
#define NORMAL_ATTRIBUTE "aVertexNormal"
#define POSITION_ATTRIBUTE "aVertexPosition"
#define TEXTURE_COORD_ATTRIBUTE "aTextureCoordinate"
#define TANGENT_ATTRIBUTE "aVertexTangent"
// Other uniforms conventions



#include "Shader.h"
#include <unordered_map>


class ShaderProgram{
private:
	std::string m_name;
	GLuint m_ID;
	std::vector<Shader> m_shaders;
	bool m_linked;	
	std::unordered_map<std::string, GLuint> m_attributesTable;
	std::unordered_map<std::string, GLuint> m_uniformTable;

	GLuint m_lightUBO;			// Remove from here. 


	void fillUniformTable();
	void fillAttributeTable();

	void addAttrib(std::string attribName);
	void addAttrib(std::vector<std::string> attribsNames); 

	GLuint addUniform(std::string uniformName);
	void addUniforms(std::vector<std::string> uniformNames);



public:

	ShaderProgram();
	ShaderProgram(std::string name);
	ShaderProgram(std::string name, std::vector<Shader> shaders);

	GLuint getID() const;
	std::string getName() const;
	Shader* getShader(shaderType type);

	void attachShader(Shader &shader);

	void link();
	bool isLinked() const;
	void use() const;
	void info() const;




	// Attribute related functions
	void enableAttrib(const std::string &attribName) const;
	void disableAttrib(const std::string &attribName) const;
	void vertexAttribPointer(const std::string &attribName, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid* data) const;

	// Uniforms related functions
	bool uniformIsAvailable(const std::string &uniformName) const;

	void setUniform1f(const std::string &uniformName, float v0) const;
	void setUniform2f(const std::string &uniformName, float v0, float v1) const;
	void setUniform3f(const std::string &uniformName, float v0, float v1, float v2) const;
	void setUniform4f(const std::string &uniformName, float v0, float v1, float v2, float v3) const;

	void setUniform1fv(const std::string &uniformName, GLsizei count, const float* value) const;
	void setUniform2fv(const std::string &uniformName, GLsizei count, const float* value) const;
	void setUniform3fv(const std::string &uniformName, GLsizei count, const float* value) const; 
	void setUniform4fv(const std::string &uniformName, GLsizei count, const float* value) const;

	void setUniformMatrix3fv(const std::string &uniformName, GLsizei count, const float* value) const;	
	void setUniformMatrix4fv(const std::string &uniformName, GLsizei count, const float* value) const;

	void setUniform1i(const std::string &uniformName, int v0) const;
	void setUniform2i(const std::string &uniformName, int v0, int v1) const;
	void setUniform3i(const std::string &uniformName, int v0, int v1, int v2) const;
	void setUniform4i(const std::string &uniformName, int v0, int v1, int v2, int v3) const;

	void setUniform1iv(const std::string &uniformName, GLsizei count, const int* value) const;
	void setUniform2iv(const std::string &uniformName, GLsizei count, const int* value) const;
	void setUniform3iv(const std::string &uniformName, GLsizei count, const int* value) const;
	void setUniform4iv(const std::string &uniformName, GLsizei count, const int* value) const;



};

#endif