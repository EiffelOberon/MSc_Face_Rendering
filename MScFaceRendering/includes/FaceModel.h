#ifndef FACE_MODEL_H
#define  FACE_MODEL_H

/*
* Refactoring notes: This is a strip down version of the Mesh class (that will change name) of cifRenderer class and a couple of stuff from the original project
*. Note that the union of this class and Geometry one is EXACTLY what was the original Model class from the  original version of this project. 
*/

#include "Geometry.h"
#include "FaceMaterial.h"

class FaceModel{

private:

	// Generic fields
	std::string m_name;

	// Geometry data
	Geometry* m_geometry;

	// Matrices
	glm::mat4 m_modelMatrix;

	// Material
	FaceMaterial* m_material;

	// Bounding Box related (spherical BB for now)
	glm::vec3 m_bbCenter;
	float m_bbRadius;


	void computeBoundingBox();

public:
	
	FaceModel();
	FaceModel(const std::string &name);
	FaceModel(const std::string &name, Geometry* geometry);
	FaceModel(const std::string &name, const std::string &filename, FaceMaterial* material);
	FaceModel(const std::string &name, Geometry* geometry, FaceMaterial* material);

	// Various getters
	std::string getName() const;
	
	glm::mat4 getModelMatrix() const;

	Geometry* getGeometry() const;	
	float getBBRadius() const;
	glm::vec3 getBBCenter() const;

	FaceMaterial* getMaterial() const;

	

	// Setters
	void setMaterial(FaceMaterial* material);
	void setGeometry(Geometry* geom);
	
	// Transformations
	void rotate(glm::vec3 angles);
	void translate(glm::vec3 displacements);


	~FaceModel();
};

#endif // !FACE_MODEL_H
