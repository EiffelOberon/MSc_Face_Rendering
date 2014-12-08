#include "../includes/Shader.h"

Shader::Shader(){}

Shader::Shader(std::string name, enum shaderType type){
	m_name = name;
	m_type = type;

	switch(type){
	case VERTEX_SHADER:
		m_ID = glCreateShader(GL_VERTEX_SHADER);
		break;
	case FRAGMENT_SHADER:
		m_ID = glCreateShader(GL_FRAGMENT_SHADER);
		break;
	case GEOMETRY_SHADER:
		m_ID = glCreateShader(GL_GEOMETRY_SHADER);
		break;
	case TESSELLETION_CONTROL_SHADER:
		m_ID = glCreateShader(GL_TESS_CONTROL_SHADER);
		break;
	case TESSELLATION_EVALUATION_SHADER:
		m_ID = glCreateShader(GL_TESS_EVALUATION_SHADER);
		break;
	case COMPUTE_SHADER:
		m_ID = glCreateShader(GL_COMPUTE_SHADER);
		break;
	default:
		printf("Bad shader type, the available are:\n"
			"VERTEX_SHADER\n FRAGMENT_SHADER\n GEOMETRY_SHADER\n TESSELLETION_CONTROL_SHADER \n  TESSELLATION_EVALUATION_SHADER\n COMPUTE_SHADER\n");
		break;

	}
}

Shader::Shader(std::string name, std::string filename, shaderType type){
	m_name = name;
	m_type = type;

	switch(type){
	case VERTEX_SHADER:
		m_ID = glCreateShader(GL_VERTEX_SHADER);
		break;
	case FRAGMENT_SHADER:
		m_ID = glCreateShader(GL_FRAGMENT_SHADER);
		break;
	case GEOMETRY_SHADER:
		m_ID = glCreateShader(GL_GEOMETRY_SHADER);
		break;
	case TESSELLETION_CONTROL_SHADER:
		m_ID = glCreateShader(GL_TESS_CONTROL_SHADER);
		break;
	case TESSELLATION_EVALUATION_SHADER:
		m_ID = glCreateShader(GL_TESS_EVALUATION_SHADER);
		break;
	case COMPUTE_SHADER:
		m_ID = glCreateShader(GL_COMPUTE_SHADER);
		break;
	default:
		printf("Bad shader type, the available are:\n"
			"VERTEX_SHADER\n FRAGMENT_SHADER\n GEOMETRY_SHADER\n TESSELLATION_SHADER\n COMPUTE_SHADER\n");
		break;

	}
	loadFromFile(filename);
}


// Get the ID of the Shader
GLuint Shader::getID() const{
	return m_ID;
}

// Get the name of the shader
std::string Shader::getName() const{
	return m_name;
}

// Get the type of the shader
enum shaderType Shader::getType() const{
	return m_type;
}

std::string Shader::getSource() const{
	return m_source;
}

// Set the source code of the shader from the input string
void Shader::loadFromString(std::string source){
	m_source = source;
	const char* tmp_src = source.c_str();
	glShaderSource(m_ID,1,&tmp_src,NULL);
	compile();
}


// Read the source code of the shader from the file whose path is given as input
void Shader::loadFromFile(std::string filename){
	std::string sourceCode;
	std::ifstream shaderStream(filename, std::ios::in);

	if(shaderStream.is_open()){
		std::string line="";
		while(std::getline(shaderStream,line)){
			sourceCode+= "\n"+line;
		}
	}else{
		std::cout<<"The file "<<filename<<" can't be opened\n";
	}
	m_source = sourceCode;
	const char* tmp_src = sourceCode.c_str();
	glShaderSource(m_ID,1,&tmp_src,NULL);
	compile();


}



void Shader::setSource(std::string source){
	const char* tmp_src = source.c_str();
	m_source = source;
	glShaderSource(m_ID, 1, &tmp_src, NULL);
	compile();
}


// Compile the shader. Return true only if no warnings or errors are thrown
bool Shader::compile(){
	int logLength;
	glCompileShader(m_ID);
	// Check if there are compilation error or warnings
	glGetShaderiv(m_ID,GL_INFO_LOG_LENGTH,&logLength);
	if(logLength!=0){ // There is indeed an error or warning
		std::vector<char> logMSG(logLength+1);
		glGetShaderInfoLog(m_ID,logLength,NULL,&logMSG[0]);
		printf("%s\n",logMSG);
		return false;
	}
	return true;

}



Shader::~Shader(){
}