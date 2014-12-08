#ifndef GEOMETRY_H
#define  GEOMETRY_H


/*
* Refactoring Note: This is straight from the cifRenderer project. The original project had all these data and functions in the Mesh class itself. It may be not strictly needed for this project as instead is in cifRenderer, but still
* makes more sense and may be strictly necessary in future development of this project with multiple faces (not done here because the project deliberately focus on a single face). 
* There are some fallacies here (no IBO support for example). I am going to solve them soon in cifRenderer and then change it here. 
*/

#include <string>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/integer.hpp"
#include <GL/glew.h>


#define FACE_SIZE 3		// This is temporary.  For the time being the project just support triangular mesh (this aspect/limitation can be found also in tessellation options)



// Used momentarily just in parsing OBJ stage. 
struct Face{
	glm::ivec4 v;
	glm::ivec4 vn;
	glm::ivec4 vt;
};

class Geometry{

private:

	std::string m_name;

	// Vertices data. Not necessarily needed, but I may want to do something with them in the future. May change it
	std::vector<glm::vec3> m_positionData;
	std::vector<glm::vec2> m_uvData;
	std::vector<glm::vec3> m_normalData;
	std::vector<glm::vec3> m_tangentData;


	// VBOs
	GLuint m_posVBO;
	GLuint m_uvVBO;
	GLuint m_normVBO;
	GLuint m_tanVBO;

	//   GLuint m_IBO; // Not use for now. Will add soon. 



	// Loading function. For now just OBJ
	void getFromOBJ(std::string filename);
	std::vector<Face> parseOBJ(std::string filename);
	void unwrapFaces(std::vector<Face> faceIndices);


	void flipVcoord();
	void generateTangents(std::vector<Face> faceIndices);
	void initVBOs();



public:

	Geometry();
	Geometry(std::string filename);


	// Read from file a mesh. For now just from OBJ
	void loadFromFile(std::string filename);

	// Getters
	std::string getName() const;
	unsigned int getNumberOfVertices() const;

	GLuint getPosVBO() const;
	GLuint getUvVBO() const;
	GLuint getNormVBO() const;
	GLuint getTanVBO() const;

	// For bounding box usage
	float getDiameter() const;			// Get the wider difference in coordinates (i.e. max(xSpan, ySpan, zSpan) )

	~Geometry();
};

#endif
