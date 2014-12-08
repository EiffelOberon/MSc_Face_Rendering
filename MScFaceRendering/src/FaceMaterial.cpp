#include "../includes/FaceMaterial.h"


FaceMaterial::FaceMaterial() : 
	m_name("nameNotSet"),
	m_rhoS(0.27f),
	m_roughness(0.27f),
	m_specD(BECKMANN),
	m_specF(SCHLICK),
	m_specG(KELEMEN),
	m_Ch(0.02f),
	m_Cm(0.002f),
	m_F0(0.028f),
	m_useTessellation(false)
{}

FaceMaterial::FaceMaterial(const std::string &name) : 
	m_name(name),
	m_rhoS(0.27f),
	m_roughness(0.27f),
	m_specD(BECKMANN),
	m_specF(SCHLICK),
	m_specG(KELEMEN),
	m_Ch(0.02f),
	m_Cm(0.002f),
	m_F0(0.028f),
	m_useTessellation(false)
{}

FaceMaterial::FaceMaterial(const std::string &name, const std::vector<renderOption> &options)  : 
	m_name(name),
	m_rhoS(0.27f),
	m_roughness(0.27f),
	m_specD(BECKMANN),
	m_specF(SCHLICK),
	m_specG(KELEMEN),
	m_Ch(0.02f),
	m_Cm(0.002f),
	m_F0(0.028f),
	m_useTessellation(false),
	m_renderOptions(options)
	
{
	activate();
}



FaceMaterial::~FaceMaterial(void){
}

/** Generate shaders **/

void FaceMaterial::generateShaderProgram(){
	m_program = ShaderProgram("Face Program");
	Shader vs = generateVS();
	Shader fs = generateFS();
	
	m_program.attachShader(vs);
	m_program.attachShader(fs);

	if(m_useTessellation){
		Shader tes = generateTES();
		Shader tcs = generateTCS();
		m_program.attachShader(tcs);
		m_program.attachShader(tes);
	}

	m_program.link();
	m_program.info();
}

// Generate the source code of the vertex shader according to the render options
Shader FaceMaterial::generateVS(){
	std::string sourceCode;
	std::ifstream shaderStream;
	if(!m_useTessellation){
		shaderStream = std::ifstream(VERTEX_FACE_SHADER, std::ios::in);
	}else{
		shaderStream = std::ifstream(VERTEX_TESS_FACE_SHADER, std::ios::in);
	}

	auto startOptions = m_renderOptions.cbegin();
	auto endOptions = m_renderOptions.cend();

	if(shaderStream.is_open()){
		std::string line="";
		std::string s;
		while(std::getline(shaderStream,line)){
			std::istringstream iss(line);
			iss >> s;
			if(s=="#define"){
				iss >> s;

				if(s=="NORMAL_MAP"){
					if((std::find(startOptions,endOptions,NORMAL_MAP)) != endOptions){
						line = "#define NORMAL_MAP 1";
					}
					else if((std::find(startOptions,endOptions,NORMAL_GEOM)) != endOptions){
						line = "#define NORMAL_MAP 0";
					}
				}
				if(s=="SHADOW"){
					if((std::find(startOptions,endOptions,SHADOW_SIMPLE)) != endOptions || 
						(std::find(startOptions,endOptions,SHADOW_PCF)) != endOptions   || 
						(std::find(startOptions,endOptions,SHADOW_VSM)) != endOptions){
						line = "#define SHADOW 1";
					}
					else{
						line = "#define SHADOW 0";
					}
				}
			}
			sourceCode+= "\n"+line;
		}
	}else{
		if(m_useTessellation){
			 std::cout<<"The file "<<VERTEX_TESS_FACE_SHADER<<" can't be opened\n";
		}else std::cout<<"The file "<<VERTEX_FACE_SHADER<<" can't be opened\n";
	}

	Shader vs("Face Shading VS", VERTEX_SHADER);
	vs.loadFromString(sourceCode);

	return vs;

}

// Generate the source code of the tessellation control shader according to the render options
Shader FaceMaterial::generateTCS(){
	std::string sourceCode;
	std::ifstream shaderStream(TCS_FACE_SHADER, std::ios::in);
	auto startOptions = m_renderOptions.cbegin();
	auto endOptions = m_renderOptions.cend();

	if(shaderStream.is_open()){
		std::string line="";
		std::string s;
		while(std::getline(shaderStream,line)){
			std::istringstream iss(line);
			iss >> s;
			if(s=="#define"){
				iss >> s;

				if(s=="OUTPUT_CP"){
					line = "#define OUTPUT_CP "+std::to_string(3);	// For the moment only triangular meshes are supported 
				}

				if(s=="SHADOW"){
					if((std::find(startOptions,endOptions,SHADOW_SIMPLE)) != endOptions || 
						(std::find(startOptions,endOptions,SHADOW_PCF)) != endOptions   || 
						(std::find(startOptions,endOptions,SHADOW_VSM)) != endOptions){
						line = "#define SHADOW 1";
					}
					else{
						line = "#define SHADOW 0";
					}
				}

				if(s=="NORMAL_MAP"){
					if((std::find(startOptions,endOptions,NORMAL_MAP)) != endOptions){
						line = "#define NORMAL_MAP 1";
					}else{
						line = "#define NORMAL_MAP 0";
					}
				}


				// NOTE_FOR_ME: Although it is set here it has not fully tested and therefore DO NOT use for now this option
				if(s=="DISTANCE_BASED"){
					if(m_distanceTess)
						line = "#define DISTANCE_BASED 1";
					else
						line = "#define DISTANCE_BASED 0";
				}
			}

			// The following set the params for the distance based tessellation
			// NOTE_FOR_ME: Although it is set here it has not fully tested and therefore DO NOT use for now this option
			if(s=="const"){
				iss >> s;
				iss >> s;
				// Note: in the shader code there must be a space between stepDistances and [] qualifier (fix this) 
				if(s == "stepDistances"){
					if(m_distanceTess){
						line = "const float stepDistances["+std::to_string(m_tessStepDistances.size())+"] = float[](";
						for(unsigned int i=0; i<m_tessStepDistances.size()-1; i++){
							line += std::to_string(m_tessStepDistances.at(i))+",";
						}
						line += std::to_string(m_tessStepDistances.at(m_tessStepDistances.size()-1))+");";
					}
				}
				// Note: in the shader code there must be a space between stepTL and [] qualifier(fix this) 
				if(s == "stepTL"){
					if(m_distanceTess){
						line = "const float stepTL["+std::to_string(m_stepTLs.size())+"] = float[](";
						for(unsigned int i=0; i<m_stepTLs.size()-1; i++){
							line += std::to_string(m_stepTLs.at(i))+",";
						}
						line += std::to_string(m_stepTLs.at(m_stepTLs.size()-1))+");";
					}
				}
			}
			sourceCode+= "\n"+line;
		}
	}else{
		std::cout<<"The file "<<TCS_FACE_SHADER<<" can't be opened\n";
	}	

	Shader tcs("Tess Control shader", TESSELLETION_CONTROL_SHADER);
	tcs.loadFromString(sourceCode);
	return tcs;
}

// Generate the source code of the tessellation evaluation shader according to the render options
Shader FaceMaterial::generateTES(){
	std::string sourceCode;
	std::ifstream shaderStream(TES_FACE_SHADER, std::ios::in);
	auto startOptions = m_renderOptions.cbegin();
	auto endOptions = m_renderOptions.cend();

	if(shaderStream.is_open()){
		std::string line="";
		std::string s;
		while(std::getline(shaderStream,line)){
			std::istringstream iss(line);
			iss >> s;
			if(s=="#define"){
				iss >> s;

				if(s=="SHADOW"){
					if((std::find(startOptions,endOptions,SHADOW_SIMPLE)) != endOptions || 
						(std::find(startOptions,endOptions,SHADOW_PCF)) != endOptions   || 
						(std::find(startOptions,endOptions,SHADOW_VSM)) != endOptions){
						line = "#define SHADOW 1";
					}
					else{
						line = "#define SHADOW 0";
					}
				}

				if(s=="NORMAL_MAP"){
					if((std::find(startOptions,endOptions,NORMAL_MAP)) != endOptions){
						line = "#define NORMAL_MAP 1";
					}
					else{
						line = "#define NORMAL_MAP 0";
					}
				}


				if(s=="OUTPUT_CP"){
					line = "#define OUTPUT_CP "+std::to_string(3);	// For the moment only triangular meshes are supported 
				}
				// Only ccw tested
				if(s=="WINDING ORDER"){
					if(m_tessCCW)
						line = "#define WINDING ORDER ccw";
					else
						line = "#define WINDING ORDER cw";
				}
				// Only equal tested
				if(s=="SPACING"){
					if(m_tessSpacing == EQUAL)
						line = "#define SPACING equal_spacing";
					else if(m_tessSpacing == FRACTIONAL_EVEN)
						line = "#define SPACING fractional_even_spacing";
					else if(m_tessSpacing == FRACTIONAL_ODD)
						line = "#define SPACING fractional_odd_spacing";						
				}
				if(s == "DISPLACEMENT"){
					if(std::find(m_renderOptions.cbegin(),m_renderOptions.cend(),DISPLACEMENT) != m_renderOptions.cend()){
						line = "#define DISPLACEMENT 1";
					}else
						line = "#define DISPLACEMENT 0";
				}
			}
			sourceCode+= "\n"+line;
		}
	}else{
		std::cout<<"The file "<<TES_FACE_SHADER<<" can't be opened\n";
	}	

	Shader tes("Tess eval shader", TESSELLATION_EVALUATION_SHADER);
	tes.loadFromString(sourceCode);

	return tes;
}

// Generate the source code of the fragment shader according to the render options
Shader FaceMaterial::generateFS(){
	std::string sourceCode;
	std::ifstream shaderStream(FRAGMENT_FACE_SHADER, std::ios::in);
	auto startOptions = m_renderOptions.cbegin();
	auto endOptions = m_renderOptions.cend();

	if(shaderStream.is_open()){
		std::string line="";
		std::string s;
		while(std::getline(shaderStream,line)){
			std::istringstream iss(line);
			iss >> s;
			if(s=="#define"){
				iss >> s;
				if(s=="NORMAL_MAP"){
					if((std::find(startOptions,endOptions,NORMAL_MAP)) != endOptions){
						line = "#define NORMAL_MAP 1";
					}
					else if((std::find(startOptions,endOptions,NORMAL_GEOM)) != endOptions){
						line = "#define NORMAL_MAP 0";
					}else{
						line = "#define NORMAL_MAP 0";	
					}
				}

				if(s=="SHADOW"){
					if((std::find(startOptions,endOptions,SHADOW_SIMPLE)) != endOptions){
						line = "#define SHADOW 1";
					
					}else if((std::find(startOptions,endOptions,SHADOW_PCF)) != endOptions){
						line = "#define SHADOW 2";

					}else if((std::find(startOptions,endOptions,SHADOW_VSM)) != endOptions){
						line = "#define SHADOW 3";

					}else{
						line = "#define SHADOW 0";
					}
				}


				if(s=="LINEAR_DEPTH"){
					if((std::find(startOptions,endOptions,LINEAR_DEPTH)) != endOptions){
						line = "#define LINEAR_DEPTH 1";
					}else{
						line = "#define LINEAR_DEPTH 0";
					}
				}



				if(s=="NO_SPEC"){
					if((std::find(startOptions,endOptions,NO_SPEC)) != endOptions){
						line = "#define NO_SPEC 1";
					}else{
						line = "#define NO_SPEC 0";
					}
				}

				if(s=="JUST_COLOR"){
					if((std::find(startOptions,endOptions,JUST_COLOR)) != endOptions){
						line = "#define JUST_COLOR 1";
					}else{
						line = "#define JUST_COLOR 0";
					}
				}

				if(s=="GAMMA"){
					if((std::find(startOptions,endOptions,INPUT_HAS_GAMMA)) != endOptions){
						line = "#define GAMMA 1";
					}else{
						line = "#define GAMMA 0";
					}
				}


				if(s=="SPEC_ONLY"){
					if((std::find(startOptions,endOptions,SPEC_ONLY)) != endOptions){
						line = "#define SPEC_ONLY 1";
					}else{
						line = "#define SPEC_ONLY 0";
					}
				}
				

				if(s=="DEBUG_NO_COL"){
					if((std::find(startOptions,endOptions,DEBUG_NO_COL)) != endOptions){
						line = "#define DEBUG_NO_COL 1";
					}else{
						line = "#define DEBUG_NO_COL 0";
					}
				}

				if(s=="DEBUG_SHOW_NORMAL"){
					if((std::find(startOptions,endOptions,DEBUG_SHOW_NORMAL)) != endOptions){
						line = "#define DEBUG_SHOW_NORMAL 1";
					}else{
						line = "#define DEBUG_SHOW_NORMAL 0";
					}
				}


				
				if(s=="DEBUG_JUST_SKIN"){
					if((std::find(startOptions,endOptions,DEBUG_JUST_SKIN)) != endOptions){
						line = "#define DEBUG_JUST_SKIN 1";
					}else{
						line = "#define DEBUG_JUST_SKIN 0";
					}
				}


				if(s=="SPECULAR"){
					if((std::find(startOptions,endOptions,SPEC_MAP)) != endOptions){
						line = "#define SPECULAR 1";
					}else if((std::find(startOptions,endOptions,SPEC_CONST)) != endOptions){
						line = "#define SPECULAR 1";
					}
				}


				if(s=="DISTRIBUTION"){
					line = "#define DISTRIBUTION "+std::to_string(m_specD);
				}

				if(s=="GEOMETRIC"){
					line = "#define GEOMETRIC "+std::to_string(m_specG);
				}

				if(s=="FRESNEL"){
					line = "#define FRESNEL "+std::to_string(m_specF);
				}

				if(s=="DISPLACEMENT_MAP"){
					if((std::find(startOptions,endOptions,DISPLACEMENT)) != endOptions){
						line = "#define DISPLACEMENT_MAP 1";
					}else{
						line = "#define DISPLACEMENT_MAP 0";
					}
				}

				if(s=="MELANINE"){
					if((std::find(startOptions,endOptions,MELANINE_MAP)) != endOptions){
						line = "#define MELANINE 2";
					}
					else if((std::find(startOptions,endOptions,MELANINE_CONST)) != endOptions){
						line = "#define MELANINE 1";
					}else{
						line = "#define MELANINE 0";
					}
				}

				if(s=="HEMOGLOBINE"){
					if((std::find(startOptions,endOptions,HEMO_MAP)) != endOptions){
						line = "#define HEMOGLOBINE 2";
					}
					else if((std::find(startOptions,endOptions,HEMO_CONST)) != endOptions){
						line = "#define HEMOGLOBINE 1";
					}else{
						line = "#define HEMOGLOBINE 0";
					}
				}

			}
			sourceCode+= "\n"+line;
		}
	}else{
		std::cout<<"The file "<<FRAGMENT_FACE_SHADER<<" can't be opened\n";
	}
	Shader fs("Skin Shading FS", FRAGMENT_SHADER);
	fs.loadFromString(sourceCode);

	return fs;
}




ShaderProgram FaceMaterial::getProgram(){
	return m_program;
}




/** Other program related **/

// This function is just an alias for activate.  With this name is easier to see its usage in some contexts
void FaceMaterial::updateShaders(){
	activate();		
}

void FaceMaterial::activate(){
	generateShaderProgram();
	uploadParams();

}



/** Texture related **/

void FaceMaterial::addTexture(const std::string &samplerName, Texture* texture){
	m_textureTable.insert(std::pair<std::string, Texture*>(samplerName, texture));
	if(!m_program.uniformIsAvailable(samplerName)){
		std::cout<<"Note that for the current program there's no sampler associated with the name"<<samplerName<<"\n";
	}

}

// Set all the uniforms for the textures. It assume the program is already in use
void FaceMaterial::uploadTextures() const{
	int textureNumber = 0;
	for(auto it=m_textureTable.begin(); it!=m_textureTable.end(); it++){
		glActiveTexture(GL_TEXTURE0+textureNumber);
		it->second->bind();
		m_program.setUniform1i(it->first,textureNumber);
		textureNumber++;
	}	
}


unsigned int FaceMaterial::getNumberOfTextures() const{
	return m_textureTable.size();
}


/** Specular setters **/

void FaceMaterial::setConstRho(float rho_s){
	m_rhoS = rho_s;
	if(m_program.isLinked()){
		if(std::find(m_renderOptions.cbegin(),m_renderOptions.cend(),SPEC_CONST) != m_renderOptions.cend()){
			m_program.setUniform1f("rho_s",m_rhoS);			
		}
	}
}

void FaceMaterial::setConstRough(float roughness){
	m_roughness = roughness;
	if(m_program.isLinked()){
		if(std::find(m_renderOptions.cbegin(),m_renderOptions.cend(),SPEC_CONST) != m_renderOptions.cend()){
			m_program.setUniform1f("roughness",m_roughness);
		}
	}
}

void FaceMaterial::setConstantSpecParam(float rho_s, float roughness){
	m_rhoS = rho_s;
	m_roughness = roughness;
	if(m_program.isLinked()){
		if(std::find(m_renderOptions.cbegin(),m_renderOptions.cend(),SPEC_CONST) != m_renderOptions.cend()){
			m_program.setUniform1f("rho_s",m_rhoS);			
			m_program.setUniform1f("roughness",m_roughness);
		}
	}
}

// Set the Distribution, Geometric and Fresnel term in the torrance-sparrow model
void FaceMaterial::setSpecularBRDF(specularD D, specularG G, specularF F){
	m_specD = D;
	m_specF = F;
	m_specG = G;
}


/** Tessellation setters **/
void FaceMaterial::setFixedTessellation(float inner, float outer){
	m_tessInner = inner;
	m_tessOuter = outer;

	if(m_program.isLinked()){
		if(std::find(m_renderOptions.cbegin(),m_renderOptions.cend(),TESSELLATION) != m_renderOptions.cend()){
			m_program.use();
			m_program.setUniform1f("TLOuter", outer);
			m_program.setUniform1f("TLInner", inner);
		}
	}

	m_distanceTess = false;
}

void FaceMaterial::setTessellationSpacing(tessellationSpacing spacingType){
	m_tessSpacing = spacingType;
}

void FaceMaterial::tesellationCCW(bool ccw){
	m_tessCCW = ccw;
}



bool FaceMaterial::useTessellation(){
	return m_useTessellation;
}


// Active the tessellation based on distance and set params for it (Not yet integrated) 
void FaceMaterial::setDistanceBasedTess(const std::vector<float> &distances, const std::vector<float> &TLsteps){
	m_tessStepDistances = distances;
	m_stepTLs = TLsteps;
	m_distanceTess = true;
}

/** Displacement Related **/

void FaceMaterial::setDisplacementFactor(float factor){
	m_displacementFactor = factor;
	if(m_program.isLinked()){
		if(std::find(m_renderOptions.cbegin(),m_renderOptions.cend(),DISPLACEMENT) != m_renderOptions.cend()){
			m_program.use();
			m_program.setUniform1f("displacementFactor", factor);
		}
	}
}

void FaceMaterial::setDisplacementZeroPoint(float zeroPoint){
	zeroPoint = glm::clamp(zeroPoint, 0.0f, 1.0f);
	m_displacementZeroPoint = zeroPoint;
	if(m_program.isLinked()){
		if(std::find(m_renderOptions.cbegin(),m_renderOptions.cend(),DISPLACEMENT) != m_renderOptions.cend()){
			m_program.use();
			m_program.setUniform1f("zeroPoint", zeroPoint);
		}
	}
}



/** Other setters **/

// Set chromophores (haemoglobin and melanin) concentration values
void FaceMaterial::setChromoConcentration(float Ch, float Cm){
	m_Ch = Ch;
	m_Cm = Cm;
	if(m_program.isLinked()){
		m_program.use();
		if(std::find(m_renderOptions.cbegin(),m_renderOptions.cend(),HEMO_CONST) != m_renderOptions.cend()){
			m_program.setUniform1f("hemConcentration",m_Ch);
		}
		if(std::find(m_renderOptions.cbegin(),m_renderOptions.cend(),MELANINE_CONST) != m_renderOptions.cend()){
			m_program.setUniform1f("melConcentration",m_Cm);
		}

	}
}


void FaceMaterial::uploadParams(){
	m_program.use();
	auto startOptions = m_renderOptions.cbegin();
	auto endOptions = m_renderOptions.cend();
	
	if(std::find(startOptions,endOptions,SPEC_MAP) == endOptions){
		m_program.setUniform1f("rho_s",m_rhoS);
		m_program.setUniform1f("roughness",m_roughness);
	}

	if(std::find(startOptions,endOptions,MELANINE_CONST) != endOptions){
		m_program.setUniform1f("melConcentration",m_Cm);
	}


	if(std::find(startOptions,endOptions,HEMO_CONST) != endOptions){
		m_program.setUniform1f("hemConcentration",m_Ch);
	}

	m_program.setUniform1f("F0", m_F0);

	if(m_useTessellation){
		m_program.setUniform1f("TLInner", m_tessInner);
		m_program.setUniform1f("TLOuter", m_tessOuter);

		if(std::find(startOptions,endOptions,DISPLACEMENT) != endOptions){
			m_program.setUniform1f("displacementFactor", m_displacementFactor);
			m_program.setUniform1f("zeroPoint", m_displacementZeroPoint);
		}

	}

}


/** Render Options related **/
void FaceMaterial::setRenderOptions(const std::vector<renderOption> &renderOptions){
	m_renderOptions = renderOptions;
	if((std::find(m_renderOptions.cbegin(),m_renderOptions.cend(),TESSELLATION)) != m_renderOptions.cend()){

		m_useTessellation = true;
		// defaults for tessellation
		glPatchParameteri(GL_PATCH_VERTICES, 3);		// Note: Only triangular mesh are supported atm (to fix in a near future)
		setFixedTessellation(7.0f, 7.0f);
		m_tessCCW = true;
		m_tessSpacing = EQUAL;
	}
	if((std::find(m_renderOptions.cbegin(),m_renderOptions.cend(),DISPLACEMENT)) != m_renderOptions.cend()){
		m_displacementFactor = 0.0f;
		m_displacementZeroPoint = 0.0f;
	}
}

void FaceMaterial::addRenderOption(renderOption renderOption){
	if(renderOption == TESSELLATION){ 
		m_useTessellation = true;
		// defaults for tessellation
		glPatchParameteri(GL_PATCH_VERTICES, 3);	// Note: Only triangular mesh are supported atm (to fix in a near future)
		setFixedTessellation(3.0f, 3.0f);
		m_tessCCW = true;
		m_tessSpacing = EQUAL;
	}
	if(renderOption == DISPLACEMENT){
		m_displacementFactor = 0.0f;
		m_displacementZeroPoint = 0.0f;
	}
	m_renderOptions.push_back(renderOption);
}

void FaceMaterial::removeRenderOption(renderOption renderOption){
	m_renderOptions.erase(std::remove(m_renderOptions.begin(), m_renderOptions.end(), renderOption), m_renderOptions.end());
	updateShaders();
}
