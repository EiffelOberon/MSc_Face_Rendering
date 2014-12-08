#ifndef RENDERER_H
#define  RENDERER_H

/*
* Notes: For the desired application only one head is rendered at time and nothing else, therefore there's no "Scene" concept and therefore
* only one mesh is rendered at a time. Note that with current structure allowing multiple meshes is trivial but the post-process sss must change (again, pretty trivially)
* As side problem of this, lights and shadows are handled here, which is something I am not in love with at all. 
*
* Due to the usage of a single mesh there are some aspects of the code (that gives perf advantages) that must be changed as soon more faces are wanted:
*	- The rendering of the shadow maps and uploading of model matrix is done only if the model matix has changed. This is done by keeping a copy of model matrix here. If expanding to multiple faces  either I do not 
*	  exploit this or keep a vector of model matrices to check everytime. If there's the guarantee that the mesh move per frame, then this makes no sense and lead to useless checks. 
*/

#include "Camera.h"
#include "RenderTarget.h"
#include "FaceModel.h"
#include "GLFW\glfw.h"

#define MAX_LIGHTS 128

enum shadowMethod {NO_SHADOW, BASIC, PCF, VSM};

#define SHADOW_MAP_VS "Shaders/Shadows/shadowMap.vert"
#define SHADOW_MAP_FS "Shaders/Shadows/shadowMap.frag"
#define VSM_PROGRAM_VS "Shaders/Shadows/VSMShadowMap.vert"
#define VSM_PROGRAM_FS "Shaders/Shadows/VSMShadowMap.frag"


// Note those are vec4 because of std140 layout
struct Light{
	glm::vec4 position;		  
	glm::vec4 lightIntensity; 
	glm::vec4 attenuationTerms;	
};



class Renderer{

private: 

	FaceModel* m_currModel;					// The only mesh to be rendered
	glm::mat4 m_currentModelMatrix;			// This is used to avoid rendering shadow maps each frame and upload the matrices only when needed. Note this trick is so easy just because we have one model
											// if the program is modified so to accept multiple faces, then this have to change (maybe a vector of model matrices, or no trick like this at all)

	// Camera related
	Camera m_camera;
	glm::ivec2 m_viewport; 
	glm::mat4 m_currPMatrix; 
	glm::mat4 m_currVMatrix; 

	// Lights
	std::vector<Light> m_lights;
	GLuint m_lightUBOID;

	// Shadow related (do not like it here)
	glm::ivec2 m_shadowViewport;
	std::vector<RenderTarget>  m_shadowMaps;
	std::vector<glm::mat4> m_lightVPMatrices; 
	ShaderProgram m_shadowProgram;
	shadowMethod m_shadowMethod;

	// VAOs
	GLuint m_VAO;						// The one generally used
	GLuint m_shadowVAO;				    // Shadows do not need anythign but positions


	// Utils shaderprogram 
	ShaderProgram m_passthrough;

	// Various
	GLuint m_screenQuadVBO;




	void initWindow(const std::string &windowname);

	void initVAOs();
	void updateVAOs();
	
	void initQuad();

	void generateLightUBOID();

	void setupPassthrough();
	
	// Lighting related
	void uploadLight(unsigned int index);
	void uploadLights();

	// Shadows related
	void setupShadowProgram();
	glm::mat4 defineLightVPMatrix(const Light &light);
	void defineLightVPMatrices();
	void updateLightVPMatrix(unsigned int index);
	void renderShadows();		


	// Rendering related
	void renderSingleLight(unsigned int index);





public:
	Renderer(void);
	Renderer(const std::string &windowname, int viewportWidth, int viewportHeight);

	~Renderer(void);


	// Misc getters
	glm::vec2 getPixelSize() const;
	Camera* getCamera();


	// Lighting related
	void addLight(const glm::vec3 &position, const glm::vec3 &intensity, const glm::vec3 &atten);
	void translateLight(unsigned int i, const glm::vec3 &translate);
	void parseEnvPreSet(const std::string &filename, float intensityScaling);


	// Model related
	void setModel(FaceModel* fm);


	// Shadows related
	void setShadowMethod(shadowMethod method);
	void setShadowMapSize(unsigned int width, unsigned int height);


	// Rendering related
	void displayRenderTarget(const RenderTarget &rt);
	void renderSceneBlendingOnRT(const RenderTarget &rt);




};

#endif // !RENDERER_H
