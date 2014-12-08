#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

/*
* Refactoring Note: The one is directly derived from the PostProcessor from the cifRenderer project which is an evolution of the original PostProcessor developed for the MSc project (started and finished before the start of cifRenderer). 
* The differences with the original PostProcessor are subtle anyway. 
*/ 

#include "RenderTarget.h"
#include "ShaderProgram.h"


// Add a define for each Post Process available
#define MSC_SSS	0									// Our take on screen space subsurface scattering 
#define JIM_SSS	1								    // Jiemenez et al. 2009 Screen Space Diffusion 
#define TONE_MAPPING 2								// Tone mapping
#define GAUSS_BLUR 3								// Not directly used by the project, but can be used for Variance Shadow Maps

// Paths to shaders
#define PASSTHROUGH_PATH_VS "Shaders/PostProcessing/passThrough.vert"

#define TONE_MAP_PATH "Shaders/PostProcessing/HDR/toneMapping.frag"
#define GAUSS_BLUR_PATH  "Shaders/PostProcessing/GaussianBlur/gaussBlur.frag"
#define JIM_SSSSS_PATH "Shaders/PostProcessing/SSSSS/SSSSSBlur_Jim.frag"
#define MSC_SSSSS_PATH "Shaders/PostProcessing/SSSSS/mySSSSS.frag"


#define SSSSS_COMBINER_PATH "Shaders/PostProcessing/SSSSS/SSSSSCombiner.frag"

enum toneMapFunction {GAMMA_CORR=0, EXPONENTIAL, REINHARD, HEJL, HABLE=4};


class PostProcessor{

private: 

	RenderTarget m_inRT;
	RenderTarget m_tmpRT;

	// Relate the Post Process (defined by the const int above defined) with the needed shaders
	std::unordered_map<int, std::vector<ShaderProgram>> m_shaderPrograms;

	// Quad buffer needed to render render targets
	GLuint m_quadVBO;
	// All the post process use basically the same VAO
	GLuint m_VAO;

	//All post process share the same vertex shader
	Shader m_passthroughVS;

	/////////// METHODS //////////////

	// Generic
	void setupPassthrough();
	void initQuadVBO();
	void initVAO();


	/*********** SSSSS Related ************/
	// Note: this is not finished thoroughly. Is a bit rough as implementation (e.g. specular is not added after and scattering is on textured) for now. Refine soon. 

	Shader generateCombineShader(const std::vector<glm::vec3> &weights);
	ShaderProgram setupCombiner(const std::vector<glm::vec3> &weights);
	void MSc_SSSCombine(const std::vector<RenderTarget> &targets) const;
	void jim_SSSCombine(const std::vector<RenderTarget> &targets) const;


	// Jimenez 2009 (6/4 Gaussians)
	void applyJimenezSSSSS(const std::vector<RenderTarget> &targets, const std::vector<float> &widths) const;

	// My MSc
	void applyMScSSSSS(const std::vector<RenderTarget> &targets, const std::vector<float> &widths, const glm::mat4 &M, const glm::mat4 &pvMatrix) const;

	/*********** Gaussian Blur Related ************/

	Shader generateGaussBlurFS(unsigned int samples);

	/*********** Tone Map Related ************/

	Shader generateToneMapFS(toneMapFunction function);
	void applyToneMap() const;


public:

	// Generic functions
	PostProcessor();
	PostProcessor(RenderTarget rt);
	PostProcessor(RenderTarget rt, GLuint quadVBO);

	void beginPostProcessing();		// It actually just bind the VAO for now. May be useful for generic ops needed by all processes. 
	void endPostProcessing();

	void setInRenderTarget(const RenderTarget& rt);
	void createTmpRT(unsigned int width, unsigned int heigth, bool HDR);
	void setQuadVBO(GLuint vbo);
	void setViewport(unsigned int width, unsigned int height);

	~PostProcessor(void);

	/////////////// EFFECTS RELATED ///////////////

	/*********** Gaussian Blur Related ************/

	void setupGaussBlur(unsigned int samples);
	void setupGaussBlur(unsigned int samples, float gaussWidth);
	void applyGaussBlurOnScreen() const;
	void applyGaussBlurOnTarget(const RenderTarget &outRT) const;
	void setGaussianWidth(float sigma);

	/*********** Tone Map Related ************/

	void setupToneMapEffect(toneMapFunction function);
	void setupToneMapEffect(toneMapFunction function, float exposure);

	void applyToneMappingOnScreen() const; 
	void applyToneMappingOnTarget(const RenderTarget &outRT) const;
	void setToneMapExposure(float exposure);


	/*********** SSSSS Related ************/

	// Jimenez et al. 2009
	void setupJimSSSSS(float sssLevel, float sssCorrection, const std::vector<glm::vec3> &weights);
	void applyJimSSSSSOnScreen(const std::vector<RenderTarget> &targets, const std::vector<float> &widths) const;
	void applyJimSSSSSOnTarget(const RenderTarget &outRT, const std::vector<RenderTarget> &targets, const std::vector<float> &widths) const;

	void setSSSlevel(float sssLevel);
	void setSSSCorrection(float correction);
	void setSSSMaxDD(float maxDD);

	// My MSc 
	void setupMScSSSSS(const std::vector<glm::vec3> &weights);
	void applyMScSSSSSOnScreen(const std::vector<RenderTarget> &targets, const std::vector<float> &widths, const glm::mat4 &M, const glm::mat4 &pvMatrix) const;
	void applyMScSSSSSOnTarget(const RenderTarget &outRT, const std::vector<RenderTarget> &targets, const std::vector<float> &widths, const glm::mat4 &M, const glm::mat4 &pvMatrix) const;

};


#endif