#include "../includes/ShaderProgram.h"

ShaderProgram::ShaderProgram(){}

ShaderProgram::ShaderProgram(std::string name) : 
	m_name(name),
	m_linked(false)
{
	m_ID = glCreateProgram();
}

ShaderProgram::ShaderProgram(std::string name, std::vector<Shader> shaders) : 
	m_name(name),
	m_linked(false)
{
	m_ID = glCreateProgram();
	for(unsigned int i=0; i<shaders.size(); i++){
		attachShader(shaders.at(i));
	}
}

GLuint ShaderProgram::getID() const{
	return m_ID;
}

std::string ShaderProgram::getName() const{
	return m_name;
}

void ShaderProgram::attachShader( Shader &shader){
	m_shaders.push_back(shader);
	glAttachShader(m_ID,shader.getID());
}

bool ShaderProgram::isLinked() const{
	return m_linked;
}


// FOR NOW THIS HAS THE FOLLOWING LIMITATIONS:
//		- Blocks can only be lighting related (easily fixable) << Quickly
//		- There's no explicit support to arrays.
// FIX ASAP
void ShaderProgram::fillUniformTable(){
	for(unsigned int i=0; i<m_shaders.size(); i++){
		std::string currSource = m_shaders.at(i).getSource();
		std::string line,s, type;
		std::istringstream sourceStream(currSource);
		while(std::getline(sourceStream,line)){
			std::istringstream iss(line);
			iss >> s;
			if(s=="layout(std140)"){
				iss >> s;	// skip keyword "uniform"
				iss >> s;
				m_lightUBO = glGetUniformBlockIndex(m_ID, s.c_str());
				glUniformBlockBinding(m_ID, m_lightUBO, LIGHT_UBO_BINDING_POINT);
			} else if(s=="uniform"){
				iss >> type; // skip type
				iss >> s;
				s = s.substr(0, s.size()-1); // Remove ;
				addUniform(s);	
			}
		}
	}
}

void ShaderProgram::fillAttributeTable(){
	addAttrib(TEXTURE_COORD_ATTRIBUTE);
	addAttrib(POSITION_ATTRIBUTE);
	addAttrib(NORMAL_ATTRIBUTE);
	addAttrib(TANGENT_ATTRIBUTE);

}

void ShaderProgram::link(){
	glLinkProgram(m_ID);
	GLint Result, InfoLogLength;
	glGetProgramiv(m_ID, GL_LINK_STATUS, &Result);
	glGetProgramiv(m_ID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(m_ID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
		printf("");
	}
	m_uniformTable.clear();
	m_attributesTable.clear();
	fillUniformTable();
	fillAttributeTable();
	m_linked = true;
}

void ShaderProgram::use() const{
	glUseProgram(m_ID);

}

void ShaderProgram::info() const{
	std::cout<<"Shader Program: "<<m_name<<"\n	Shader Attached:\n";
	for(unsigned int i=0; i<m_shaders.size(); i++){
		std::cout<<"		- "<<m_shaders.at(i).getName()<<"\n";
	}
	std::cout<<"Available attributes:\n ";
	for(auto it=m_attributesTable.begin(); it!=m_attributesTable.end(); it++){
		if(it->second!=-1)
			std::cout<<"	- "<<it->first<<"	........	ID = "<<it->second<<"\n";
	}
	std::cout<<"Available uniforms:\n ";
	for(auto it=m_uniformTable.begin(); it!=m_uniformTable.end(); it++){
		std::cout<<"	- "<<it->first<<"	........	ID = "<<it->second<<"\n";
	}
	std::cout<<"\n";

	std::cout<<"UBO ID: "<<m_lightUBO<<"\n"; 
}


void ShaderProgram::addAttrib(std::string attribName){
	GLuint attribLocation = glGetAttribLocation(m_ID,attribName.c_str());
	if(attribLocation!=-1){
		m_attributesTable.insert(std::pair<std::string, GLuint>(attribName,attribLocation));
	}else{
#if VERBOSE
		std::cout<<"Attribute "<<attribName<<" not found in shader. Check for it in shader code\n";
#endif
	}
}

void ShaderProgram::addAttrib(std::vector<std::string> attribNames){
	for(unsigned int i=0; i<attribNames.size(); i++){
		addAttrib(attribNames.at(i));
	}
}

void ShaderProgram::disableAttrib(const std::string &attribName) const{
	auto it = m_attributesTable.find(attribName);
	if(it == m_attributesTable.end()){
#if VERBOSE
		std::cout<<"Attribute "<<attribName<<" not found in shader. Check for it in shader code\n";
#endif
	}else
		glDisableVertexAttribArray(it->second);
}

void ShaderProgram::enableAttrib(const std::string &attribName) const{
	auto it = m_attributesTable.find(attribName);
	if(it == m_attributesTable.end()){
#if VERBOSE
		std::cout<<"Attribute "<<attribName<<" not found in shader. Check for it in shader code\n";
#endif
	}else
		glEnableVertexAttribArray(it->second);
}

void ShaderProgram::vertexAttribPointer(const std::string &attribName, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid* data) const{
	auto it = m_attributesTable.find(attribName);
	if(it == m_attributesTable.end()){
#if VERBOSE
		std::cout<<"Attribute "<<attribName<<" not found in shader. Check for it in shader code\n";
#endif
	}else{
		GLuint attribLocation = it->second;
		glVertexAttribPointer(attribLocation,size,type,normalized,stride,data);
	}

}


GLuint ShaderProgram::addUniform(std::string uniformName){
	const char* tmpUnifName = uniformName.c_str();
	GLuint uniformLocation = glGetUniformLocation(m_ID,tmpUnifName);
	if(uniformLocation!=-1){
		m_uniformTable.insert(std::pair<std::string, GLuint>(uniformName,uniformLocation));
	}else{
#if VERBOSE
		std::cout<<"No uniform with name "<< uniformName<<" found in any of the shaders attached to this program OR the uniform is not use in the code.\n";
#endif	
	}
	return uniformLocation;
}

void ShaderProgram::addUniforms(std::vector<std::string> uniformNames){
	for(unsigned int i=0; i<uniformNames.size(); i++){
		addUniform(uniformNames.at(i));
	}
}


void ShaderProgram::setUniform1f(const std::string &uniformName, float v0) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniform1f(it->second,v0);
	}
}

void ShaderProgram::setUniform2f(const std::string &uniformName, float v0, float v1) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniform2f(it->second,v0,v1);
	}
}

void ShaderProgram::setUniform3f(const std::string &uniformName, float v0, float v1, float v2) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniform3f(it->second,v0,v1,v2);
	}
}

void ShaderProgram::setUniform4f(const std::string &uniformName, float v0, float v1, float v2, float v3) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniform4f(it->second,v0,v1,v2,v3);
	}
}

void ShaderProgram::setUniform1i(const std::string &uniformName, int v0) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniform1i(it->second,v0);
	}
}

void ShaderProgram::setUniform2i(const std::string &uniformName, int v0, int v1) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniform2i(it->second,v0,v1);
	}
}

void ShaderProgram::setUniform3i(const std::string &uniformName, int v0, int v1, int v2) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniform3i(it->second,v0,v1,v2);
	}
}

void ShaderProgram::setUniform4i(const std::string &uniformName, int v0, int v1, int v2, int v3) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniform4i(it->second,v0,v1,v2,v3);
	}
}

void ShaderProgram::setUniform1fv(const std::string &uniformName, GLsizei count, const float* value) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniform1fv(it->second,count, value);
	}
}

void ShaderProgram::setUniform2fv(const std::string &uniformName, GLsizei count, const float* value) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniform2fv(it->second,count, value);
	}
}

void ShaderProgram::setUniform3fv(const std::string &uniformName, GLsizei count, const float* value) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniform3fv(it->second,count, value);
	}
}

void ShaderProgram::setUniform4fv(const std::string &uniformName, GLsizei count, const float* value) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniform4fv(it->second,count, value);
	}
}

void ShaderProgram::setUniformMatrix3fv(const std::string &uniformName, GLsizei count, const float* value) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniformMatrix3fv(it->second,count,GL_FALSE,value);
	}
}

void ShaderProgram::setUniformMatrix4fv(const std::string &uniformName, GLsizei count, const float* value) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniformMatrix4fv(it->second,count,GL_FALSE,value);
	}
}


void ShaderProgram::setUniform1iv(const std::string &uniformName, GLsizei count, const int* value) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniform1iv(it->second,count, value);
	}
}


void ShaderProgram::setUniform2iv(const std::string &uniformName, GLsizei count, const int* value) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniform2iv(it->second,count, value);
	}
}

void ShaderProgram::setUniform3iv(const std::string &uniformName, GLsizei count, const int* value) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniform3iv(it->second,count, value);
	}
}

void ShaderProgram::setUniform4iv(const std::string &uniformName, GLsizei count, const int* value) const{
	auto it = m_uniformTable.find(uniformName);
	if(it == m_uniformTable.end()){
#if VERBOSE
		std::cout<<"No uniform found with name: "<<uniformName<<"\n";
#endif
	}else{
		glUniform4iv(it->second,count, value);
	}
}

Shader* ShaderProgram::getShader(shaderType type){
	for(unsigned int i=0; i<m_shaders.size(); i++){
		if(m_shaders.at(i).getType() == type)
			return &m_shaders.at(i);
	}
	return nullptr;
}

bool ShaderProgram::uniformIsAvailable(const std::string &uniformName) const{
	return (m_uniformTable.find(uniformName)!=m_uniformTable.end());
}




