#include "../includes/PostProcessor.h"

// Generic methods

PostProcessor::PostProcessor(){
	initQuadVBO();
	setupPassthrough();
	initVAO();
}

PostProcessor::PostProcessor(RenderTarget rt){
	initQuadVBO();
	initVAO();
	setupPassthrough();
	m_inRT = rt;
	glm::ivec2 inSize = rt.getSize();
	// Don't really like this... 
	m_tmpRT = RenderTarget(inSize.x, inSize.y, GL_RGBA, GL_RGBA, true);
}

PostProcessor::PostProcessor(RenderTarget rt, GLuint quadVBO){
	setupPassthrough();
	m_inRT = rt;
	glm::ivec2 inSize = rt.getSize();
	// Don't really like this... 
	m_tmpRT = RenderTarget(inSize.x, inSize.y, GL_RGBA, GL_RGBA, true);
	m_quadVBO = quadVBO;
	initVAO();

}

void PostProcessor::setupPassthrough(){
	m_passthroughVS = Shader("passthrough VS", PASSTHROUGH_PATH_VS, VERTEX_SHADER);

}

void PostProcessor::initQuadVBO(){
	const GLfloat quadPositions[] = { 
		-1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
	};

	glGenBuffers(1,&m_quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER,m_quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadPositions),quadPositions,GL_STATIC_DRAW);

}

void PostProcessor::setQuadVBO(GLuint vbo){
	m_quadVBO = vbo;
}

void PostProcessor::initVAO(){
	// Note: As the passthrough is default and make no sense rewriting it for now
	// it is fine to assume that 0 = pos attribute. May change this, but not now. 
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);

	glBindVertexArray(0);	// bind default VAO

}

void PostProcessor::createTmpRT(unsigned int width, unsigned int heigth, bool hdr){
	m_tmpRT = RenderTarget(width, heigth, GL_RGBA, GL_RGBA, hdr);
}

void PostProcessor::setInRenderTarget(const RenderTarget &rt){
	m_inRT = rt;
}

void PostProcessor::beginPostProcessing(){
	glBindVertexArray(m_VAO);
}

void PostProcessor::endPostProcessing(){
	glBindVertexArray(0);
}


/////////////// EFFECTS RELATED ///////////////


/*********** Tone Map Related ************/
Shader PostProcessor::generateToneMapFS(toneMapFunction function){
	std::string source = ""; 
	std::ifstream shaderStream(TONE_MAP_PATH, std::ios::in);
	if(shaderStream.is_open()){
		std::string line, s;
		while(std::getline(shaderStream,line)){
			std::istringstream iss(line);
			iss>>s;
			if(s=="#define"){
				iss>>s;
				if(s=="TONE_MAPPER"){
					line = "#define TONE_MAPPER "+std::to_string(function);
				}
			}
			source+=line+"\n";
		}
	}

	Shader toneMapShader("ToneMapping FS", FRAGMENT_SHADER);
	toneMapShader.loadFromString(source);

	return toneMapShader;
}

void PostProcessor::applyToneMap() const{
	std::vector<ShaderProgram> programs = m_shaderPrograms.find(TONE_MAPPING)->second;
	ShaderProgram toneMapping = programs.at(0);

	toneMapping.use();


	glActiveTexture(GL_TEXTURE0);
	m_inRT.bindBuffer(0);
	glDrawArrays(GL_QUADS, 0, 4);
}

void PostProcessor::setupToneMapEffect(toneMapFunction function){
	Shader toneMapper = generateToneMapFS(function);
	ShaderProgram toneMappingProgram("Tone Mapper");
	toneMappingProgram.attachShader(m_passthroughVS);
	toneMappingProgram.attachShader(toneMapper);

	toneMappingProgram.link(); 
	toneMappingProgram.info();
	toneMappingProgram.use();
	toneMappingProgram.setUniform1f("exposure", 1.0f);
	toneMappingProgram.setUniform1i("colTexture", 0);	// This is possible as colTexture will always be in TEXTURE0

	std::vector<ShaderProgram> programs; 
	programs.push_back(toneMappingProgram);
	m_shaderPrograms.insert(std::pair<const int, std::vector<ShaderProgram>>(TONE_MAPPING, programs));


}

void PostProcessor::setupToneMapEffect(toneMapFunction function, float exposure){
	setupToneMapEffect(function);
	setToneMapExposure(exposure);
}

void PostProcessor::applyToneMappingOnScreen() const{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	applyToneMap();
}

void PostProcessor::applyToneMappingOnTarget(const RenderTarget &outRT) const{
	outRT.bind();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	applyToneMap();

}

void PostProcessor::setToneMapExposure(float exposure){
	std::vector<ShaderProgram> programs = m_shaderPrograms.find(TONE_MAPPING)->second;
	ShaderProgram toneMapping = programs.at(0);

	toneMapping.use();

	toneMapping.setUniform1f("exposure", exposure);
}



/*********** Gaussian Blur Related ************/


Shader PostProcessor::generateGaussBlurFS(unsigned int samples){
	std::string source = ""; 
	std::ifstream shaderStream(GAUSS_BLUR_PATH, std::ios::in);
	// BIG ASSUMPTION HERE! I am assuming that all the render target fed will have same size (in term of px)
	glm::ivec2 rtSize = m_inRT.getSize();	
	glm::vec2 pixelSizes = glm::vec2(1.0f/rtSize.x, 1.0f/rtSize.y);


	if(samples != 5 && samples != 7 && samples != 9){
		std::cout<<"Invalid taps number. Choose 5, 7 or 9\n";
		exit(EXIT_FAILURE);
	}

	if(shaderStream.is_open()){
		std::string line,s;
		while(std::getline(shaderStream,line)){
			std::istringstream iss(line);
			iss>>s;
			if(s == "#define"){
				iss>>s;
				if(s == "SAMPLE_NUMBER"){
					source += "#define SAMPLE_NUMBER "+std::to_string(samples)+"\n";
				}
				else if(s == "PIXELSIZE"){
					source +=  "const vec2 pixelSizes = vec2("+std::to_string(pixelSizes.x)+","
						+std::to_string(pixelSizes.y)+");";
				}
			}else{
				source+=line+"\n";
			}
		}
	}

	Shader gaussBlur("Gaussian Blur 1D", FRAGMENT_SHADER);
	gaussBlur.loadFromString(source);
	return gaussBlur;

}

void PostProcessor::setupGaussBlur(unsigned int samples, float gaussWidth){
	Shader blurShader = generateGaussBlurFS(samples);
	ShaderProgram gaussProgram("Gaussian Blur");
	gaussProgram.attachShader(m_passthroughVS);
	gaussProgram.attachShader(blurShader);
	gaussProgram.link();
	gaussProgram.info();

	gaussProgram.use();
	gaussProgram.setUniform1f("gaussWidth",gaussWidth);
	gaussProgram.setUniform1i("colTexture", 0); // This is possible as colTexture will always be in TEXTURE0

	std::vector<ShaderProgram> gaussPrograms; 
	gaussPrograms.push_back(gaussProgram);
	m_shaderPrograms.insert(std::pair<const int, std::vector<ShaderProgram>>(GAUSS_BLUR, gaussPrograms));	
}

void PostProcessor::setupGaussBlur(unsigned int samples){
	setupGaussBlur(samples, 1.0f);
}

void PostProcessor::applyGaussBlurOnScreen() const{
	std::vector<ShaderProgram> programs = m_shaderPrograms.find(GAUSS_BLUR)->second;
	ShaderProgram gaussBlur = programs.at(0);

	m_tmpRT.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gaussBlur.use();

	glActiveTexture(GL_TEXTURE0);
	m_inRT.bindBuffer(0);


	// First pass
	gaussBlur.setUniform2fv("direction", 1, &(glm::vec2(1.0,0.0)[0]));
	glDrawArrays(GL_QUADS, 0, 4);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Second pass
	m_tmpRT.bindBuffer(0);

	gaussBlur.setUniform2fv("direction", 1, &(glm::vec2(0.0,1.0)[0]));
	glDrawArrays(GL_QUADS, 0, 4);

}

void PostProcessor::applyGaussBlurOnTarget(const RenderTarget &outRT) const{
	std::vector<ShaderProgram> programs = m_shaderPrograms.find(GAUSS_BLUR)->second;
	ShaderProgram gaussBlur = programs.at(0);

	m_tmpRT.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gaussBlur.use();

	glActiveTexture(GL_TEXTURE0);
	m_inRT.bindBuffer(0);

	// First pass
	gaussBlur.setUniform2fv("direction", 1, &(glm::vec2(1.0,0.0)[0]));
	glDrawArrays(GL_QUADS, 0, 4);

	outRT.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Second pass
	m_tmpRT.bindBuffer(0);
	gaussBlur.setUniform2fv("direction", 1, &(glm::vec2(0.0,1.0)[0]));
	glDrawArrays(GL_QUADS, 0, 4);

}

void PostProcessor::setGaussianWidth(float sigma){
	std::vector<ShaderProgram> programs = m_shaderPrograms.find(GAUSS_BLUR)->second;
	ShaderProgram gaussBlur = programs.at(0);
	gaussBlur.use();
	gaussBlur.setUniform1f("gaussWidth",sigma);

}



/*********** Subsurface Scattering Related ************/

Shader PostProcessor::generateCombineShader(const std::vector<glm::vec3> &weights){
	std::string source = ""; 
	std::ifstream shaderStream(SSSSS_COMBINER_PATH, std::ios::in);

	if(shaderStream.is_open()){
		std::string line;
		while(std::getline(shaderStream,line)){
			if(line == "#version 330"){
				unsigned int count = weights.size();
				source += line+"\n\n";
				source += "const int textureCount = "+std::to_string(count)+"; \n\n"; 
				source += "const float weights["+std::to_string(count*3)+"] = float[](";
				for(unsigned int i=0; i<count-1; i++){
					for(unsigned int j=0; j<3; j++){
						source+=std::to_string(weights.at(i)[j])+", ";
					}
				}
				for(unsigned int j=0; j<2; j++){
					source+=std::to_string(weights.at(count-1)[j])+", ";
				}

				source+=std::to_string(weights.at(count-1)[2])+");\n\n";
				// The space after [ is a bit of an hack to allow fillUniformTable to work as always, may be necessary to reconsider this and add specific support to arrays
				source+="uniform sampler2D textures[ "+std::to_string(count)+" ];\n\n\n";
			}else{
				source+=line+"\n";
			}
		}
	}

	Shader combinerShader("SSSSS Combiner FS",FRAGMENT_SHADER);
	combinerShader.loadFromString(source);


	return combinerShader;

}

ShaderProgram PostProcessor::setupCombiner(const std::vector<glm::vec3> &weights){

	ShaderProgram combinerProgram("Combiner Program");
	combinerProgram.attachShader(m_passthroughVS);
	combinerProgram.attachShader(generateCombineShader(weights));

	combinerProgram.link(); 
	combinerProgram.info();

	return combinerProgram;
}

void PostProcessor::MSc_SSSCombine(const std::vector<RenderTarget> &targets) const{

	ShaderProgram combiner = m_shaderPrograms.find(MSC_SSS)->second.at(1);

	combiner.use();
	std::vector<int> samplerList; 
	for(unsigned int i=0; i<targets.size(); i++){
		glActiveTexture(GL_TEXTURE0+i);
		targets.at(i).bindBuffer(0);
		samplerList.push_back(i);
	}

	combiner.setUniform1iv("textures",targets.size(),&samplerList[0]);
	glDrawArrays(GL_QUADS,0,4);
}

void PostProcessor::jim_SSSCombine(const std::vector<RenderTarget> &targets) const{

	ShaderProgram combiner = m_shaderPrograms.find(JIM_SSS)->second.at(1);

	combiner.use();
	std::vector<int> samplerList; 
	for(unsigned int i=0; i<targets.size(); i++){
		glActiveTexture(GL_TEXTURE0+i);
		targets.at(i).bindBuffer(0);
		samplerList.push_back(i);
	}

	combiner.setUniform1iv("textures",targets.size(),&samplerList[0]);
	glDrawArrays(GL_QUADS,0,4);
}


// Jimenez et al. 2009

void PostProcessor::setupJimSSSSS(float sssLevel, float sssCorrection, const std::vector<glm::vec3> &weights){

	glm::ivec2 rtSize = m_inRT.getSize();		
	glm::vec2 pixelSizes = glm::vec2(1.0f/rtSize.x, 1.0f/rtSize.y);

	Shader blurPassShader("SSSSS Blur pass FS", JIM_SSSSS_PATH, FRAGMENT_SHADER);

	ShaderProgram blurPass("Jim SSSSS Blur pass X");

	blurPass.attachShader(m_passthroughVS);
	blurPass.attachShader(blurPassShader);


	blurPass.link(); blurPass.info();

	blurPass.use();
	blurPass.setUniform1f("sssLevel", sssLevel);
	blurPass.setUniform1f("correction", sssCorrection);
	blurPass.setUniform1f("maxdd", 0.001f);
	blurPass.setUniform2fv("pixelSizes", 1, &pixelSizes[0]);
	blurPass.setUniform1i("colourTexture",0); // Will always be in TEXTURE0
	blurPass.setUniform1i("depthTexture",1);  // Will always be in TEXTURE1

	std::vector<ShaderProgram> SSSSSPrograms;
	SSSSSPrograms.push_back(blurPass); 
	SSSSSPrograms.push_back(setupCombiner(weights));

	auto it = m_shaderPrograms.find (JIM_SSS);
	m_shaderPrograms.erase (it);

	m_shaderPrograms.insert(std::pair<const int, std::vector<ShaderProgram>>(JIM_SSS, SSSSSPrograms));


}

void PostProcessor::applyJimenezSSSSS(const std::vector<RenderTarget> &targets, const std::vector<float> &widths) const{
	std::vector<ShaderProgram> programs = m_shaderPrograms.find(JIM_SSS)->second;
	ShaderProgram blurPass = programs.at(0); 

	RenderTarget firstTarget = targets.at(0);
	for(unsigned int i=1; i<targets.size(); i++){
		RenderTarget currRT = targets.at(i);

		// First Pass
		m_tmpRT.bind();
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		blurPass.use();
		blurPass.setUniform2fv("direction", 1, &(glm::vec2(1.0,0.0)[0]));
		blurPass.setUniform1f("gaussWidth", widths.at(i));

		glActiveTexture(GL_TEXTURE0);
		firstTarget.bindBuffer(0);

		glActiveTexture(GL_TEXTURE1);
		firstTarget.bindDepthBuffer();

		glDrawArrays(GL_QUADS, 0, 4);

		// Second Pass
		currRT.bind();
		glActiveTexture(GL_TEXTURE0);
		m_tmpRT.bindBuffer(0);

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		blurPass.setUniform2fv("direction", 1, &(glm::vec2(0.0,1.0)[0]));

		glDrawArrays(GL_QUADS, 0, 4);
	}

}

void PostProcessor::applyJimSSSSSOnScreen(const std::vector<RenderTarget> &targets, const std::vector<float> &widths) const{
	applyJimenezSSSSS(targets, widths);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	jim_SSSCombine(targets);

}

void PostProcessor::applyJimSSSSSOnTarget(const RenderTarget &outRT, const std::vector<RenderTarget> &targets, const std::vector<float> &widths) const{
	applyJimenezSSSSS(targets, widths);
	outRT.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	jim_SSSCombine(targets);

}

void PostProcessor::setSSSlevel(float sssLevel){
	ShaderProgram blurPass = m_shaderPrograms.find(JIM_SSS)->second.at(0);

	blurPass.use();
	blurPass.setUniform1f("sssLevel", sssLevel);
}

void PostProcessor::setSSSCorrection(float correction){
	ShaderProgram blurPass = m_shaderPrograms.find(JIM_SSS)->second.at(0);

	blurPass.use();
	blurPass.setUniform1f("correction", correction);

}

void PostProcessor::setSSSMaxDD(float maxDD){
	ShaderProgram blurPass = m_shaderPrograms.find(JIM_SSS)->second.at(0);

	blurPass.use();
	blurPass.setUniform1f("maxdd", maxDD);
}


// My MSc (version 0.1b) 
void PostProcessor::setupMScSSSSS(const std::vector<glm::vec3> &weights){

	glm::ivec2 rtSize = m_inRT.getSize();		
	glm::vec2 pixelSizes = glm::vec2(1.0f/rtSize.x, 1.0f/rtSize.y);

	Shader blurPassShader("MSc SSSSS Blur pass FS", MSC_SSSSS_PATH, FRAGMENT_SHADER);

	ShaderProgram blurPass("MSc SSSSS Blur pass ");

	blurPass.attachShader(m_passthroughVS);
	blurPass.attachShader(blurPassShader);


	blurPass.link(); blurPass.info();


	blurPass.use();
	blurPass.setUniform2fv("pixelSizes", 1, &pixelSizes[0]);
	blurPass.setUniform1i("colourTexture",0); // Will always be in TEXTURE0
	blurPass.setUniform1i("depthTexture",1);  // Will always be in TEXTURE1


	std::vector<ShaderProgram> SSSSSPrograms;
	SSSSSPrograms.push_back(blurPass); 
	SSSSSPrograms.push_back(setupCombiner(weights));

	m_shaderPrograms.insert(std::pair<const int, std::vector<ShaderProgram>>(MSC_SSS, SSSSSPrograms));


}

// TODOs:
//    Optimize both here (think of number of shaders needed, uploading of params etc.) and in shader code
//    Expand it so it works with multiple objects (here and in shaders)
//    Expand it so it works only on selected items (via alpha). In shaders both of SSSSS and Material one
void PostProcessor::applyMScSSSSS(const std::vector<RenderTarget> &targets, const std::vector<float> &widths, const glm::mat4 &M, const glm::mat4 &pvMatrix) const{

	std::vector<ShaderProgram> programs = m_shaderPrograms.find(MSC_SSS)->second;
	ShaderProgram  blurPass = programs.at(0); 

	RenderTarget firstTarget = targets.at(0);

	// Think a way to upload matrices just when necessary. 
	blurPass.use();
	blurPass.setUniformMatrix4fv("invMVP",1,&(glm::inverse(pvMatrix*M)[0][0]));

	for(unsigned int i=1; i<targets.size(); i++){
		RenderTarget currRT = targets.at(i);

		m_tmpRT.bind();
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		blurPass.use();
		blurPass.setUniform2fv("direction", 1, &(glm::vec2(1.0,0.0)[0]));
		blurPass.setUniform1f("gaussWidth", widths.at(i));

		glActiveTexture(GL_TEXTURE0);
		firstTarget.bindBuffer(0);

		glActiveTexture(GL_TEXTURE1);
		firstTarget.bindDepthBuffer();

		glDrawArrays(GL_QUADS, 0, 4);

		// Second Pass
		currRT.bind();
		glActiveTexture(GL_TEXTURE0);
		m_tmpRT.bindBuffer(0);

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		blurPass.setUniform2fv("direction", 1, &(glm::vec2(0.0,1.0)[0]));

		glDrawArrays(GL_QUADS, 0, 4);

	}
}


void PostProcessor::applyMScSSSSSOnScreen(const std::vector<RenderTarget> &targets, const std::vector<float> &widths, const glm::mat4 &M, const glm::mat4 &pvMatrix) const{
	applyMScSSSSS(targets, widths, M, pvMatrix);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	MSc_SSSCombine(targets);

}

void PostProcessor::applyMScSSSSSOnTarget(const RenderTarget &outRT, const std::vector<RenderTarget> &targets, const std::vector<float> &widths, const glm::mat4 &M, const glm::mat4 &pvMatrix) const{
	applyMScSSSSS(targets, widths, M, pvMatrix);
	outRT.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	MSc_SSSCombine(targets);

}






PostProcessor::~PostProcessor(){
	
}

















