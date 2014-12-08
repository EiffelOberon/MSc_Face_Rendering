#include "../includes/Geometry.h"


Geometry::Geometry(){   }

Geometry::Geometry(std::string filename){
	m_name = filename;
	loadFromFile(filename);
}



// File -> Geometry data
void Geometry::loadFromFile(std::string filename){
	if(filename.substr( filename.length() - 3 ) == "obj"){
		getFromOBJ(filename);
	}

	initVBOs();

}

void Geometry::getFromOBJ(std::string filename){
	std::vector<Face> faceIndices = parseOBJ(filename);
	generateTangents(faceIndices);
	flipVcoord();
	unwrapFaces(faceIndices);
}

std::vector<Face> Geometry::parseOBJ(std::string filename){

	// FACE SIZE NOW CONSTANT FIXED HERE AS 3. TO BE CHANGED
	std::vector<Face> faceIndices;

	std::string line, s; 
	std::ifstream file(filename);


	if(file.is_open()){
		while(std::getline(file,line)){
			std::istringstream iss(line);
			s.clear();
			iss >> s;

			if(s==("v")){
				glm::vec3 v;
				for(unsigned int i=0; i<3; i++){
					iss >> s;
					v[i] = (float)atof(s.data());
				}
				m_positionData.push_back(v);
			}

			else if(s==("vt")){
				glm::vec2 vt;
				for(unsigned int i=0; i<2; i++){
					iss >> s;
					vt[i] = (float)atof(s.data());
				}
				m_uvData.push_back(vt);
			}

			else if(s==("vn")){
				glm::vec3 vn;
				for(unsigned int i=0; i<3; i++){
					iss >> s;
					vn[i] = (float)atof(s.data());
				}
				m_normalData.push_back(vn);
			}

			else if(s==("f")){
				Face f;
				iss >> s;
				int numberOfField = std::count(s.begin(), s.end(), '/');

				switch (numberOfField){
				case 0:
					for(unsigned int vertexNumber=0; vertexNumber<FACE_SIZE; vertexNumber++){
						f.v[vertexNumber++] = atoi(s.data())-1;
						iss >> s;
					}
					break;
				case 1:
					for(unsigned int vertexNumber=0; vertexNumber<FACE_SIZE; vertexNumber++){
						std::istringstream blockISS(s);
						std::string value;
						getline(blockISS,value,'/');
						f.v[vertexNumber] = atoi(value.data())-1;
						getline(blockISS,value,'/');
						f.vt[vertexNumber] = atoi(value.data())-1;
						iss >> s;
					}
					break;
				case 2:
					for(unsigned int vertexNumber=0; vertexNumber<FACE_SIZE; vertexNumber++){
						std::istringstream blockISS(s);
						std::string value;
						getline(blockISS,value,'/');
						f.v[vertexNumber] = atoi(value.data())-1;
						getline(blockISS,value,'/');
						if(value.length()!=0)
							f.vt[vertexNumber] = atoi(value.data())-1;
						getline(blockISS,value,'/');
						f.vn[vertexNumber] = atoi(value.data())-1;
						iss>>s;						
					}
					break;
				}

				faceIndices.push_back(f);

			}
		}
	}

	file.close();

	return faceIndices;
}

void Geometry::unwrapFaces(std::vector<Face> faceIndices){
	std::vector<glm::vec3> newPositions;
	std::vector<glm::vec3> newNormals;
	std::vector<glm::vec2> newUVs;
	std::vector<glm::vec3> newTangents;


	for(unsigned int i=0; i<faceIndices.size(); i++){
		Face currentFace = faceIndices.at(i);
		for(unsigned int j=0; j<FACE_SIZE; j++){
			glm::vec3 position = m_positionData.at(currentFace.v[j]);
			glm::vec2 UV = m_uvData.at(currentFace.vt[j]);
			newPositions.push_back(position); 
			newUVs.push_back(UV);
			if(m_normalData.size()!=0){
				glm::vec3 normal = m_normalData.at(currentFace.vn[j]);
				newNormals.push_back(normal);
			}
			if(m_tangentData.size()!=0){
				newTangents.push_back(m_tangentData.at(currentFace.v[j]));
			}
		}
	}
	m_positionData = newPositions;
	m_uvData = newUVs;
	m_normalData = newNormals;
	m_tangentData = newTangents;

}


// Mesh pre-processing

// It assumes triangles!
// Based on Lengyel, Eric. "Computing Tangent Space Basis Vectors for an Arbitrary Mesh".
// Terathon Software 3D Graphics Library, 2001. http://www.terathon.com/code/tangent.html
void Geometry::generateTangents(std::vector<Face> faceIndices){


	if(m_normalData.size() != 0 && m_uvData.size() != 0){
		// Fill with empty vec
		for(unsigned int i=0; i<faceIndices.size(); i++){
			m_tangentData.push_back(glm::vec3(0.0f));
			m_tangentData.push_back(glm::vec3(0.0f));
			m_tangentData.push_back(glm::vec3(0.0f));

		}

		for(unsigned int i=0; i<faceIndices.size(); i++){
			Face currentFace = faceIndices.at(i);
			glm::vec3 v0 = m_positionData.at(currentFace.v[0]);
			glm::vec3 v1 = m_positionData.at(currentFace.v[1]);
			glm::vec3 v2 = m_positionData.at(currentFace.v[2]);

			glm::vec2 uv0 = m_uvData.at(currentFace.vt[0]);
			glm::vec2 uv1 = m_uvData.at(currentFace.vt[1]);
			glm::vec2 uv2 = m_uvData.at(currentFace.vt[2]);


			glm::vec3 edge1 = v1 - v0; 
			glm::vec3 edge2 = v2 - v0;

			float deltaU1 = uv1.x - uv0.x;
			float deltaU2 = uv2.x - uv0.x;

			float deltaV1 = uv1.y - uv0.y;
			float deltaV2 = uv2.y - uv0.y;

			float f = 1.0f/(deltaU1*deltaV2 - deltaU2*deltaV1);

			glm::vec3 tangent; 
			tangent.x = f*(deltaV2*edge1.x - deltaV1*edge2.x);
			tangent.y = f*(deltaV2*edge1.y - deltaV1*edge2.y);
			tangent.z = f*(deltaV2*edge1.z - deltaV1*edge2.z);

			m_tangentData.at(currentFace.v[0]) += tangent;
			m_tangentData.at(currentFace.v[1]) += tangent;
			m_tangentData.at(currentFace.v[2]) += tangent;
		}


		for(unsigned int i=0; i<m_tangentData.size(); i++){
			m_tangentData.at(i) = glm::normalize(m_tangentData.at(i));
		}
	}else{
		std::cout<<"Tangent data could not be generated due to lack of UV OR Normals data\n";
	}
}

void Geometry::flipVcoord(){
	for(unsigned int i=0; i<m_uvData.size(); i++){
		m_uvData.at(i).y = 1.0f - m_uvData.at(i).y;
	}
}


// Init all the VBOs. Note that this method assume that the Mesh has pos, normals, uv and tangents. May reconsider it. 
void Geometry::initVBOs(){

	// Pos VBO
	glGenBuffers(1, &m_posVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*m_positionData.size(), &(m_positionData[0]), GL_STATIC_DRAW);

	// UV VBO
	glGenBuffers(1, &m_uvVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_uvVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*m_uvData.size(), &(m_uvData[0]), GL_STATIC_DRAW);

	// Normal VBO
	glGenBuffers(1, &(m_normVBO));
	glBindBuffer(GL_ARRAY_BUFFER, m_normVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*m_normalData.size(), &(m_normalData[0]), GL_STATIC_DRAW);

	// Tangent VBO
	glGenBuffers(1, &m_tanVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_tanVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*m_tangentData.size(), &(m_tangentData[0]), GL_STATIC_DRAW);

}


// Getters
std::string Geometry::getName() const{
	return m_name;
}

GLuint Geometry::getPosVBO() const{
	return m_posVBO;
}

GLuint Geometry::getUvVBO() const{
	return m_uvVBO;
}

GLuint Geometry::getNormVBO() const{
	return m_normVBO;
}

GLuint Geometry::getTanVBO() const{
	return m_tanVBO;
}

unsigned int Geometry::getNumberOfVertices() const{
	return m_positionData.size();
}


float Geometry::getDiameter() const{

	glm::vec3 maxCoords, minCoords;

	for(unsigned int i=0; i<m_positionData.size(); i++){
		// x coord
		if(m_positionData.at(i).x > maxCoords.x)  maxCoords.x = m_positionData.at(i).x;
		if(m_positionData.at(i).x < minCoords.x)  minCoords.x = m_positionData.at(i).x;
		// y coord
		if(m_positionData.at(i).y > maxCoords.y)  maxCoords.y = m_positionData.at(i).y;
		if(m_positionData.at(i).y < minCoords.y)  minCoords.y = m_positionData.at(i).y;
		// z coord
		if(m_positionData.at(i).z > maxCoords.z)  maxCoords.z = m_positionData.at(i).z;
		if(m_positionData.at(i).z < minCoords.z)  minCoords.z = m_positionData.at(i).z;
	}

	glm::vec3 distances = maxCoords - minCoords;
	return std::max(distances.x, std::max(distances.y, distances.z));
}


Geometry::~Geometry(){
	glDeleteBuffers(1, &m_posVBO);
	glDeleteBuffers(1, &m_tanVBO);
	glDeleteBuffers(1, &m_normVBO);
	glDeleteBuffers(1, &m_uvVBO);
}
