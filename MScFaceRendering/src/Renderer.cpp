#include "../includes/Renderer.h"


Renderer::Renderer(void)
{
}

Renderer::Renderer(const std::string &windowname, int viewportWidth, int viewportHeight) : 
	m_viewport(glm::ivec2(viewportWidth, viewportHeight)),
	m_currPMatrix(glm::mat4(1.0)),
	m_currVMatrix(glm::mat4(1.0)),
	m_currentModelMatrix(glm::mat4(0.0))
{
	m_camera  = Camera();
	m_camera.setAspectRatio((float)m_viewport.y/m_viewport.x);

	initWindow(windowname);
	setupPassthrough();
	initQuad();
	generateLightUBOID();

	m_shadowViewport = m_viewport;
	m_shadowMethod = PCF;
	setupShadowProgram();
}


Renderer::~Renderer(void){
	// Destroy currModel here? 
}

void Renderer::initWindow(const std::string &windowname){
	//Initialise GLFW
	if( !glfwInit() ){
		fprintf( stderr, "Failed to initialize GLFW\n" );
	}

	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 0);



	// Open a window and create its OpenGL context
	if( !glfwOpenWindow( m_viewport.x, m_viewport.y, 0,0,0,0, 8,0, GLFW_WINDOW ) ){
		fprintf( stderr, "Failed to open GLFW window\n" );
		glfwTerminate();
	}

	glfwSetWindowTitle(windowname.c_str());

	glfwEnable( GLFW_STICKY_KEYS );
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glClearColor(0.0,0.0,0.0,1.0);

	glEnable(GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA) ;
	glEnable(GL_POLYGON_OFFSET_FILL);


	//	 Initialize GLEW
	glewExperimental = false; // Needed for core profile
	if (glewInit() != GLEW_OK){
		fprintf(stderr, "Failed to initialize GLEW\n");
	}

	std::cout<<"Opened context with version: "<<glGetString(GL_VERSION)<<"\n";

}



void Renderer::generateLightUBOID(){
	glGenBuffers(1,&m_lightUBOID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_lightUBOID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Light)*MAX_LIGHTS,NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER,LIGHT_UBO_BINDING_POINT,m_lightUBOID);
}




// VAO related
void Renderer::initVAOs(){

	ShaderProgram faceProgram = m_currModel->getMaterial()->getProgram();
		
	// Init the vaos that use all the attributes. 
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_currModel->getGeometry()->getPosVBO());
	faceProgram.enableAttrib(POSITION_ATTRIBUTE);
	faceProgram.vertexAttribPointer(POSITION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, m_currModel->getGeometry()->getNormVBO());
	faceProgram.enableAttrib(NORMAL_ATTRIBUTE);
	faceProgram.vertexAttribPointer(NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, m_currModel->getGeometry()->getUvVBO());
	faceProgram.enableAttrib(TEXTURE_COORD_ATTRIBUTE);
	faceProgram.vertexAttribPointer(TEXTURE_COORD_ATTRIBUTE, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, m_currModel->getGeometry()->getTanVBO());
	faceProgram.enableAttrib(TANGENT_ATTRIBUTE);
	faceProgram.vertexAttribPointer(TANGENT_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// VAO for shadows
	glGenVertexArrays(1, &m_shadowVAO);
	glBindVertexArray(m_shadowVAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_currModel->getGeometry()->getPosVBO());
	faceProgram.enableAttrib(POSITION_ATTRIBUTE);
	faceProgram.vertexAttribPointer(POSITION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	
	glBindVertexArray(0);
}


void Renderer::updateVAOs(){
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteVertexArrays(1, &m_shadowVAO);

	initVAOs();
}


// Init a VBO containing a screen aligned quad
void Renderer::initQuad(){
	const float quadPositions[] = { 
		-1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
	};

	glGenBuffers(1,&m_screenQuadVBO);
	glBindBuffer(GL_ARRAY_BUFFER,m_screenQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadPositions),quadPositions,GL_STATIC_DRAW);

}

// Setup the program used to display a render target (shaders source hard-coded here)
void Renderer::setupPassthrough(){
	std::string sourceVS = "#version 330 \n"
		"layout(location=0) in vec3 aVertexPosition;\n"
		"out vec2 vTextureCoord;\n"
		"void main(void){\n"
		"	gl_Position = vec4(aVertexPosition,1.0);\n"
		"	vTextureCoord = (aVertexPosition.xy+vec2(1,1))/2.0;\n"
		"}";
	std::string sourceFS = "#version 330 \n"
		"in vec2 vTextureCoord;\n"
		"out vec4 fragmentColor;\n"
		"uniform sampler2D rtSampler;\n"
		"void main(void){\n"	
		"fragmentColor = texture(rtSampler, vTextureCoord);\n"
		"}";

	std::cout<<"Set-upping passthrough program... \n";

	Shader passthroughVS("Passthrough VS",VERTEX_SHADER);
	passthroughVS.loadFromString(sourceVS);
	Shader texDisplayFS("Texture display FS", FRAGMENT_SHADER);
	texDisplayFS.loadFromString(sourceFS);

	m_passthrough = ShaderProgram("RT display"); 
	m_passthrough.attachShader(passthroughVS);
	m_passthrough.attachShader(texDisplayFS);
	m_passthrough.link();

}


// Setup the program used to render shadow maps 
void Renderer::setupShadowProgram(){
	Shader vs;
	Shader fs;

	if(m_shadowMethod == VSM){

		vs = Shader("Shadow VSM VS", VERTEX_SHADER);
		fs = Shader("Shadow  VSM FS", FRAGMENT_SHADER);

		vs.loadFromFile(VSM_PROGRAM_VS);
		fs.loadFromFile(VSM_PROGRAM_FS);
	}else{
		vs = Shader("Shadow VS", VERTEX_SHADER);
		fs = Shader("Shadow FS", FRAGMENT_SHADER);

		vs.loadFromFile(SHADOW_MAP_VS);
		fs.loadFromFile(SHADOW_MAP_FS);
	}
	m_shadowProgram = ShaderProgram("Shadow Map Builder");
	m_shadowProgram.attachShader(vs); 
	m_shadowProgram.attachShader(fs);
	m_shadowProgram.link();
	m_shadowProgram.info();
}

// Creates the view-projection matrix for the passed light
glm::mat4 Renderer::defineLightVPMatrix(const Light &light){
	Camera shadowCam = Camera();
	shadowCam.setPosition(glm::vec3(light.position));
	shadowCam.setTarget(m_currModel->getBBCenter());

	float distance = glm::distance(m_currModel->getBBCenter(), shadowCam.getCameraPosition());
	shadowCam.adaptFOV(m_currModel->getBBRadius(),distance);
	float nearClip = std::abs(distance - 1.20f*m_currModel->getBBRadius());
	shadowCam.setZClip(std::max(nearClip, 0.0001f), 8.0f);	// Far clip hard coded, CHANGE!

	const glm::mat4 vMatrix = shadowCam.getViewMatrix();
	const glm::mat4 pMatrix = shadowCam.getPMatrix();

	return pMatrix * vMatrix;
}


// Creates the view projection matrices for all the lights
void Renderer::defineLightVPMatrices(){
	m_lightVPMatrices.clear();
	for(unsigned int i=0; i<m_lights.size(); i++){
		m_lightVPMatrices.push_back(defineLightVPMatrix(m_lights.at(i)));
	}
}

void Renderer::updateLightVPMatrix(unsigned int index){
	m_lightVPMatrices.at(index) = defineLightVPMatrix(m_lights.at(index));
}


void Renderer::renderShadows(){
	glViewport(0, 0, m_shadowViewport.x -1, m_shadowViewport.y -1);
	m_shadowProgram.use();
	glBindVertexArray(m_shadowVAO);
	for(unsigned int i=0; i<m_shadowMaps.size(); i++){
		m_shadowMaps.at(i).bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_shadowProgram.setUniformMatrix4fv("mvpMatrix", 1, &(m_lightVPMatrices.at(i)*m_currModel->getModelMatrix())[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, m_currModel->getGeometry()->getNumberOfVertices());
	}
	glBindVertexArray(0);
	glViewport(0,0,m_viewport.x,m_viewport.y);
}


void Renderer::setShadowMapSize(unsigned int width, unsigned int height){
	m_shadowViewport = glm::ivec2(width, height);
}


void Renderer::setShadowMethod(shadowMethod method){
	m_shadowMethod = method;
}





void Renderer::addLight(const glm::vec3 &position, const glm::vec3 &intensity, const glm::vec3 &atten){
	Light light; 
	light.position = glm::vec4(position,1.0f);
	light.lightIntensity = glm::vec4(intensity,1.0f);
	light.lightIntensity = glm::vec4(atten, 0.0f);
	m_lights.push_back(light);

	m_lightVPMatrices.push_back(defineLightVPMatrix(m_lights.at(m_lights.size()-1)));
	if(m_shadowMethod == VSM){
		RenderTarget vsmRT = RenderTarget(m_shadowViewport.x,m_shadowViewport.y, GL_RGBA16F, GL_RGBA, true, true,false,true);
		vsmRT.setDepthBuffer(GL_DEPTH_COMPONENT24);
		m_shadowMaps.push_back(vsmRT);
	}else{
		m_shadowMaps.push_back(RenderTarget(m_shadowViewport.x, m_shadowViewport.y, GL_DEPTH_COMPONENT24, false));
	}

	uploadLight(m_lights.size() - 1);
}

void Renderer::translateLight(unsigned int i, const glm::vec3 &displacement){
	m_lights.at(i).position += glm::vec4(displacement,0.0);
	updateLightVPMatrix(i);
	renderShadows();
	uploadLight(i);
}


// Push the light in the right slot of the UBO
void Renderer::uploadLight(unsigned int index){
	glBindBuffer(GL_UNIFORM_BUFFER, m_lightUBOID);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Light)*index,sizeof(Light), &m_lights.at(index));

}

// Upload all the light in the UBO
void Renderer::uploadLights(){
	glBindBuffer(GL_UNIFORM_BUFFER, m_lightUBOID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Light)*m_lights.size(), &m_lights,  GL_STATIC_DRAW);
}



// THIS IS PRETTY TERRIBLE. Must change. 
void Renderer::parseEnvPreSet(const std::string &filename, float intensityScaling){
	int count = 0;
	std::string source = ""; 
	std::ifstream fileStream(filename, std::ios::in);

	if(fileStream.is_open()){
		std::string line,s;
		while(std::getline(fileStream,line)){
			count++;
			std::istringstream iss(line);
			glm::vec3 tmpDir;
			glm::vec3 tmpCol;
			// Direction:
			iss >> s; // skip D:
			iss >> s;
			tmpDir.x = (float) std::atof(s.c_str());
			iss >> s;
			tmpDir.y = (float) std::atof(s.c_str());
			iss >> s;
			tmpDir.z = (float) std::atof(s.c_str());
			std::getline(fileStream,line);
			iss = std::istringstream(line);
			iss >> s; // skip C:
			iss >> s;
			tmpCol.x = (float) std::atof(s.c_str());
			iss >> s;
			tmpCol.y = (float) std::atof(s.c_str());
			iss >> s;
			tmpCol.z = (float) std::atof(s.c_str());
			float lum = 0.2126f*tmpCol.r + 0.7152f*tmpCol.g +  0.0722f*tmpCol.b;
			if(tmpDir.z > 0.0)
				tmpDir.z += 1.0f;
			else
				tmpDir.z -=1.0f;

			Light l;
			l.position = glm::vec4(tmpDir, 1.0);
			l.lightIntensity = glm::vec4(tmpCol*intensityScaling, 0.0);
			l.attenuationTerms = glm::vec4(1.0f, 0.1f, 0.01f, 0.0f);
			m_lights.push_back(l);
		}
	}

	for(unsigned int i=0; i<m_lights.size(); i++){
		m_lightVPMatrices.push_back(defineLightVPMatrix(m_lights.at(i)));
		if(m_shadowMethod == VSM){
			RenderTarget vsmRT = RenderTarget(m_shadowViewport.x,m_shadowViewport.y, GL_RGBA16F, GL_RGBA, true, true,false,true);
			vsmRT.setDepthBuffer(GL_DEPTH_COMPONENT32);
			m_shadowMaps.push_back(vsmRT);
		}else{
			m_shadowMaps.push_back(RenderTarget(m_shadowViewport.x, m_shadowViewport.y, GL_DEPTH_COMPONENT32, false));
		}

		uploadLight(i);
	}

}



void Renderer::setModel(FaceModel* fm){
	m_currModel = fm;
	updateVAOs();
	defineLightVPMatrices();	// because Bounding Box info
}



// Misc getters. 
glm::vec2 Renderer::getPixelSize() const{
	return glm::vec2(1.0) / (glm::vec2) m_viewport;
}

Camera* Renderer::getCamera(){
	return &m_camera;
}

// Render the scene with a single light (it will be blended). The right program is already in use
void Renderer::renderSingleLight(unsigned int lightIndex){

	ShaderProgram sp = m_currModel->getMaterial()->getProgram();

	sp.setUniform1i("lightIndex", lightIndex);

	if(m_shadowMethod != NO_SHADOW){
		sp.setUniformMatrix4fv("depthVP", 1, &(m_lightVPMatrices.at(lightIndex))[0][0]);
		unsigned int textureUnit = m_currModel->getMaterial()->getNumberOfTextures();
		glActiveTexture(GL_TEXTURE0+textureUnit+1);
		m_shadowMaps.at(lightIndex).bindDepthBuffer();
		sp.setUniform1i("shadowTexture", textureUnit+1);
	}
	if(m_currModel->getMaterial()->useTessellation()){
		glDrawArrays(GL_PATCHES, 3, m_currModel->getGeometry()->getNumberOfVertices());
	}else{
		glDrawArrays(GL_TRIANGLES, 0, m_currModel->getGeometry()->getNumberOfVertices());
	}
}

void Renderer::renderSceneBlendingOnRT(const RenderTarget &rt){

	ShaderProgram sp = m_currModel->getMaterial()->getProgram();

	bool modelMatrixChanged = (m_currentModelMatrix != m_currModel->getModelMatrix());
	bool viewMatrixChanged = (m_currVMatrix != m_camera.getViewMatrix());
	bool pMatrixChanged = (m_currPMatrix != m_camera.getPMatrix());

	if(modelMatrixChanged && m_shadowMethod != NO_SHADOW){
		renderShadows();
	}


	rt.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	sp.use();

	m_currModel->getMaterial()->uploadTextures();
	
	if(m_shadowMethod != NO_SHADOW){
		glm::vec2 shadowPixelSize = glm::vec2(1.0f) / (glm::vec2) m_shadowViewport;
		sp.setUniform2fv("shadowPixelSize", 1, &(glm::vec2(shadowPixelSize)[0]));			// Actually not needed here, here for clarity, move when confident
	}
	
	if(modelMatrixChanged){
		m_currentModelMatrix = m_currModel->getModelMatrix();
		sp.setUniformMatrix4fv("mMatrix", 1, &(m_currentModelMatrix)[0][0]);
		glm::mat3 nMatrix = glm::mat3(glm::inverse(glm::transpose(m_currentModelMatrix)));
		sp.setUniformMatrix3fv("nMatrix", 1, &nMatrix[0][0]);

	}

	if(viewMatrixChanged){
		m_currVMatrix = m_camera.getViewMatrix();
		sp.setUniformMatrix4fv("vMatrix", 1, &(m_currVMatrix)[0][0]);
	}

	if(pMatrixChanged){
		m_currPMatrix = m_camera.getPMatrix();
		sp.setUniformMatrix4fv("pMatrix", 1, &(m_currPMatrix)[0][0]);
	}



	glBindVertexArray(m_VAO);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);
	renderSingleLight(0);

	glBlendFunc(GL_ONE, GL_ONE);
	glDepthFunc(GL_LEQUAL);

	for(unsigned int i=1; i<m_lights.size(); i++){
		renderSingleLight(i);
	}

	glBindVertexArray(0);
	glDisable(GL_BLEND);	
}


void Renderer::displayRenderTarget(const RenderTarget &rt){
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_passthrough.use();

	glEnableVertexAttribArray(0);

	glActiveTexture(GL_TEXTURE0);
	rt.bindBuffer(0);
	m_passthrough.setUniform1f("rtSampler",0);

	glBindBuffer(GL_ARRAY_BUFFER, m_screenQuadVBO);
	m_passthrough.vertexAttribPointer(POSITION_ATTRIBUTE,3,GL_FLOAT,GL_FALSE,0,(void*)0);
	glDrawArrays(GL_QUADS,0,4);

	glDisableVertexAttribArray(0);

}


