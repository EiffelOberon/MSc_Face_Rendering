#include "../includes/FaceModel.h"


FaceModel::FaceModel(void)
{}


FaceModel::FaceModel(const std::string &name) : 
	m_name(name),
	m_modelMatrix(glm::mat4(1.0))
{

}


FaceModel::FaceModel(const std::string &name, Geometry* geometry) :
	m_name(name),
	m_modelMatrix(glm::mat4(1.0)),
	m_geometry(geometry)
{
	computeBoundingBox();
}

FaceModel::FaceModel(const std::string &name, const std::string &filename, FaceMaterial* material) : 
	m_name(name),
	m_modelMatrix(glm::mat4(1.0)),
	m_material(material)
{

	m_geometry = new Geometry(filename);
	computeBoundingBox();
}

FaceModel::FaceModel(const std::string &name, Geometry* geometry, FaceMaterial* material) : 
	m_name(name),
	m_modelMatrix(glm::mat4(1.0)),
	m_material(material),
	m_geometry(geometry)
{
	computeBoundingBox();
}


FaceModel::~FaceModel(void){}



// Getters
std::string FaceModel::getName() const{
	return m_name;
}

glm::mat4 FaceModel::getModelMatrix() const{
	return m_modelMatrix;
}



FaceMaterial* FaceModel::getMaterial() const{
	return m_material;
}

Geometry* FaceModel::getGeometry() const{
	return m_geometry;
}

void FaceModel::computeBoundingBox(){
	m_bbRadius = m_geometry->getDiameter() /  2.0f;
	// Note the center at the beginning is 0,0,0 then it is moved at each translation
	m_bbCenter = glm::vec3(0.0f);		// change this line to use model matrix
}


glm::vec3 FaceModel::getBBCenter() const{
	return m_bbCenter;
}

float FaceModel::getBBRadius() const{
	return m_bbRadius;
}

// Setters
void FaceModel::setMaterial(FaceMaterial* material){
	m_material = material;
}

void FaceModel::setGeometry(Geometry* geom){
	m_geometry = geom;
}



// Transformations
void FaceModel::rotate(glm::vec3 angles){
	if(angles.x != 0.0f)
		m_modelMatrix = glm::rotate(m_modelMatrix,angles.x, glm::vec3(1.0f,0.0f,0.0f));
	if(angles.y != 0.0f)
		m_modelMatrix = glm::rotate(m_modelMatrix,angles.y, glm::vec3(0.0f,1.0f,0.0f));
	if(angles.z != 0.0f)
		m_modelMatrix = glm::rotate(m_modelMatrix,angles.z, glm::vec3(0.0f,0.0f,1.0f));
}

void FaceModel::translate(glm::vec3 displacement){
	m_modelMatrix = glm::translate(m_modelMatrix, displacement);
	// Move centre
	m_bbCenter += displacement;
}
