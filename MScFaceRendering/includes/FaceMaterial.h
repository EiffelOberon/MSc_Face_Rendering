#ifndef FACE_MATERIAL_H
#define  FACE_MATERIAL_H

/*
* This class is used to define and keep the state and characteristic of the main pass shading (i.e. before the sss post-processing step) for a Face.
* This can be seen as the hub to set the option for such main pass
* It is very specific as the project is pretty specific. 
*/

/*
* NOTE: Throughout the code some uniform names are hard-coded here as the shaders are fixed. This is something I'd like to change 
* in a near future. 
*/

#include "ShaderProgram.h"
#include "Texture.h"
#include <algorithm>

// The shaders used have a fixed location on the disk 
#define VERTEX_FACE_SHADER "Shaders/Face/FaceShader.vert"
#define VERTEX_TESS_FACE_SHADER "Shaders/Face/FaceShaderTess.vert"
#define FRAGMENT_FACE_SHADER "Shaders/Face/FaceShader.frag"
#define TCS_FACE_SHADER "Shaders/Face/FaceShader.tcs"
#define TES_FACE_SHADER "Shaders/Face/FaceShader.tes"



enum renderOption {NORMAL_MAP,		// Use normal maps
	NORMAL_GEOM,					// Use the per-vertex normal in the Mesh
	SPEC_CONST,						// The specular params are constant over all the face
	SPEC_MAP,						// The specular params (rho_s and roughness) are obtained from a texture
	SPEC_ONLY,						// It is sort of a debug option. Only the specular component of lighting is shown
	MELANINE_MAP,					// Melanin values are used and taken from an apposite map
	MELANINE_CONST,					// A constant value is used for melanin (it doesn't make sense for a final result, useful to debug skin tones) 
	HEMO_MAP,						// Haemoglobin values are used and taken from an apposite map
	HEMO_CONST,						// A constant value is used for haemoglobin (it doesn't make sense for a final result, useful to debug skin tones) 
	SHADOW_SIMPLE,					// Shadows are used and the shadow test is a simple one
	SHADOW_PCF,						// Shadows are used and PCF is used
	SHADOW_VSM,						// Variance Shadow Maps are used.	
	INPUT_HAS_GAMMA,				// If set, the values in the textures are elevated to 2.2 
	TESSELLATION,					// If set, tessellation is applied 
	DISPLACEMENT,					// If set, displacement take place	(a displacement map must be set AND for time being, also TESSELLATION MUST be set)
	JUST_COLOR,						// No lighting is used, just diffuse colour is returned (It's sort of a debug option)
	NO_SPEC,						// No specular lighting is computed (It's sort of a debug option)
	LINEAR_DEPTH,					// The depth buffer is written as linear (may be useful for some post-processing) 
	DEBUG_NO_COL, DEBUG_JUST_SKIN, DEBUG_SHOW_NORMAL,		// DEBUG OPTIONs (some are redundant, this is going to go away anyway)
}; 
enum specularD { BLINN = 0, GGX, BECKMANN };
enum specularG { IMPLICIT = 0, NEUMANN, COOK_TORRANCE, KELEMEN, WALTER, SMITH_SCHLICK};
enum specularF { NONE = 0, COOK_TORRANCE_F, SCHLICK }; 

enum tessellationSpacing {EQUAL, FRACTIONAL_EVEN, FRACTIONAL_ODD};		


class FaceMaterial{

private:

	std::string m_name;
	std::vector<renderOption> m_renderOptions;
	std::unordered_map<std::string, Texture*> m_textureTable;
	ShaderProgram m_program;
	
	/** Specular Related **/
	float m_rhoS;							// Used only if specular params are constant across the whole face
	float m_roughness;						// Used only if specular params are constant across the whole face
	float m_F0;
	// Options for the Torrance-Sparrow model (See above for the available options)
	specularD m_specD;
	specularG m_specG;
	specularF m_specF;

	/*** Chromophores related ***/
	// Used only if distribution constant all over the skin.
	float m_Ch;								// Haemoglobin concentration
	float m_Cm;								// Melanin concentration

	/*** Tessellation Related ***/
	bool m_useTessellation;
	tessellationSpacing m_tessSpacing;
	float m_tessInner;
	float m_tessOuter;
	bool m_tessCCW;
	// Tessellation based on distance related stuff (not fully tested, therefore not a render option yet and not integrated)
	bool m_distanceTess;					// If the tessellation is based distances
	std::vector<float> m_tessStepDistances; // The step that triggers newer levels of tessellation
	std::vector<float> m_stepTLs;			// The tesellation levels 

	/*** Displacement Related ***/
	float m_displacementFactor;
	float m_displacementZeroPoint;

//////////////// Methdos ////////////////
	void generateShaderProgram();
	Shader generateVS();
	Shader generateFS();
	Shader generateTES();
	Shader generateTCS();





public:

	FaceMaterial();
	FaceMaterial(const std::string &name);
	FaceMaterial(const std::string &name, const std::vector<renderOption> &renderOptions);


	ShaderProgram getProgram();

	/** Texture related **/
	void addTexture(const std::string &samplerName, Texture* texture); 
	void uploadTextures() const;
	unsigned int getNumberOfTextures() const;


	void setChromoConcentration(float Ch, float Cm);

	/** Specular related **/
	void setConstantSpecParam(float rho_s, float roughness);
	void setConstRho(float rho_s);
	void setConstRough(float roughness);
	void setF0(float F0);
	void setSpecularBRDF(specularD, specularG, specularF);


	/** Tessellation related **/
	void setFixedTessellation(float innerLevel , float outerLevel);
	void setTessellationSpacing(tessellationSpacing spacingType);
	void tesellationCCW(bool ccw);
	void setDistanceBasedTess(const std::vector<float> &distances, const std::vector<float> &TLsteps);	// Not yet fully tested and therefore not integrated yet
	bool useTessellation();
	
	/** Dispalcement related **/
	void setDisplacementFactor(float factor); 
	void setDisplacementZeroPoint(float zeroPoint);

	/** Rendering option related **/
	void setRenderOptions(const std::vector<renderOption> &renderOptions);
	void addRenderOption(renderOption renderOption);
	void removeRenderOption(renderOption renderOption);


	void updateShaders();
	void uploadParams();
	void activate();


	~FaceMaterial();
};

#endif // !FACE_MATERIAL_H
