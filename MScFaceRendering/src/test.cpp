#include "../includes/PostProcessor.h"
#include "../includes/Renderer.h"




void DEon6GoF(std::vector<glm::vec3> *W, std::vector<float> *widths){
	W->push_back(glm::vec3(0.233,0.455,0.649));
	W->push_back(glm::vec3(0.1,0.336,0.344));
	W->push_back(glm::vec3(0.118,0.198,0.0));
	W->push_back(glm::vec3(0.113,0.007,0.007));
	W->push_back(glm::vec3(0.358,0.004,0.0));
	W->push_back(glm::vec3(0.078,0.0,0.0));

	widths->push_back(0.08f);
	widths->push_back(0.22f);
	widths->push_back(0.4324f);
	widths->push_back(0.753f);
	widths->push_back(1.41f);
	widths->push_back(2.72f);
}



void main(void){

/**************************************************************************************/
	Renderer renderer("Test window",720,720);

	// Set the material and its rendering options
	FaceMaterial fm("Face Material");
	fm.addRenderOption(NORMAL_MAP);
	fm.addRenderOption(SHADOW_PCF);
	fm.addRenderOption(INPUT_HAS_GAMMA);
	fm.addRenderOption(SPEC_MAP);

	fm.activate();

	// Load the needed textures and add them to the material 
	Texture diffTexture("Textures/Map-COL.jpg");
	Texture normalMap("Textures/tangentNormalResized.jpg");
	Texture specular("Textures/Map-sp.jpg");

	fm.addTexture("normalTexture", &normalMap);
	fm.addTexture("diffuseTexture", &diffTexture);
	fm.addTexture("specParamTex", &specular);


	fm.uploadParams();





	Geometry geom("Models/head.obj");
	FaceModel model("Face", &geom, &fm);

	// Set the data needed by the renderer
	renderer.setModel(&model);
	renderer.getCamera()->advance(0.3f);
	renderer.getCamera()->setTarget(glm::vec3(0.0f));
	renderer.parseEnvPreSet("EnvPresets/preSetUffizi3108_16.txt", 0.001f);

	// Set the post-processor options

	// Related to SSS
	std::vector<RenderTarget> targets;
	std::vector<glm::vec3> W;
	std::vector<float> widths;
	for(unsigned int i=0; i<6; i++){
		RenderTarget rt = RenderTarget(720,720, GL_RGBA16F, GL_RGBA, true);
		rt.setDepthBuffer(GL_DEPTH_COMPONENT24);
		targets.push_back(rt);
	}
	DEon6GoF(&W, &widths);

	PostProcessor pp = PostProcessor(targets.at(0));
	pp.setupMScSSSSS(W);
	pp.setupToneMapEffect(HEJL, 1.0);



	// To control exposure
	float exp = 1.0f;





	// This render target would serve as "tmp" RT between post-processes. Not strictly necessary.
	RenderTarget outRT(720, 720, GL_RGBA16F, GL_RGBA, true);
	outRT.setDepthBuffer(GL_DEPTH_COMPONENT24);
	
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	while (true){
		renderer.renderSceneBlendingOnRT(targets.at(0));
		pp.beginPostProcessing();
		pp.setInRenderTarget(targets.at(0));
		pp.applyMScSSSSSOnTarget(outRT, targets, widths, model.getModelMatrix(), renderer.getCamera()->getPMatrix() * renderer.getCamera()->getViewMatrix());
		pp.setInRenderTarget(outRT);
		pp.applyToneMappingOnScreen();
		pp.endPostProcessing();



		if(glfwGetKey('I')==GLFW_PRESS){
			renderer.getCamera()->advance(-0.04f);
		}
		if(glfwGetKey('K')==GLFW_PRESS){
			renderer.getCamera()->advance(0.04f);
		}

		if(glfwGetKey('Q')==GLFW_PRESS){
			model.rotate(glm::vec3(0.0,1.0,0.0));
		}

		if(glfwGetKey('W')==GLFW_PRESS){
			model.rotate(glm::vec3(0.0,-1.0,0.0));
		}


		if(glfwGetKey('E')==GLFW_PRESS){
			exp += 0.01f;
			pp.setToneMapExposure(exp);
		}

		if(glfwGetKey('C')==GLFW_PRESS){
			exp -= 0.01f;
			pp.setToneMapExposure(exp);
		}


		// Measure perf
		double currentTime = glfwGetTime();
		nbFrames++;
		if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1 sec ago
			// printf and reset timer
			printf("%f ms/frame\n", 1000.0/double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		glfwSwapBuffers();
	}
}
//}
