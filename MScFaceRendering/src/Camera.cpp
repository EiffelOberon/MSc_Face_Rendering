#include "../includes/Camera.h"


Camera::Camera() : 
	m_position(glm::vec3(0.0f)), 
	m_upVector(glm::vec3(0.0f,1.0f,0.0f)),
	m_forwardVector(glm::vec3(0.0,0.0,-1.0)),
	m_FOV(45.0f),
	m_nearClip(0.01f),
	m_farClip(100.0f),
	m_aspectRatio(1.0f)
{
	m_targeting = false;
	m_position = glm::vec3(0.0f);
	m_upVector = glm::vec3(0.0f,1.0f,0.0f);
	m_forwardVector = glm::vec3(0.0f,0.0f,-1.0f);
	computeViewMatrix();
	computePerspectiveMatrix();
}

Camera::Camera(glm::vec3 position, glm::vec3 forwardVector) : 
	m_position(position), 
	m_upVector(glm::vec3(0.0f,1.0f,0.0f)),
	m_forwardVector(forwardVector),
	m_FOV(45.0f),
	m_nearClip(0.01f),
	m_farClip(100.0f),
	m_aspectRatio(1.0f)
{
	m_targeting = false;
	m_rightVector = glm::cross(m_upVector, m_forwardVector);
	computeViewMatrix();
	computePerspectiveMatrix();

}

Camera::Camera(glm::vec3 position, glm::vec3 upVector, glm::vec3 forwardVector) : 
	m_position(position), 
	m_upVector(upVector),
	m_forwardVector(forwardVector),
	m_FOV(45.0f),
	m_nearClip(0.01f),
	m_farClip(100.0f),
	m_aspectRatio(1.0f)
{
	m_targeting = false;
	m_rightVector = glm::cross(m_upVector, m_forwardVector);
	computeViewMatrix();
	computePerspectiveMatrix();
}

Camera::Camera(glm::vec3 position, glm::vec3 upVector, glm::vec3 forwardVector, float FOV, float aspect, float nearZ, float farZ):
	m_position(position), 
	m_upVector(upVector),
	m_forwardVector(forwardVector),
	m_FOV(FOV),
	m_nearClip(nearZ),
	m_farClip(farZ),
	m_aspectRatio(aspect)
{
	m_targeting = false;
	m_rightVector = glm::cross(m_upVector, m_forwardVector);
	computeViewMatrix();
	computePerspectiveMatrix();
}

void Camera::translateCamera(const glm::vec3 &displacement){
	m_position += displacement;
	computeViewMatrix();
}

void Camera::rotateCameraPosition(const glm::vec3 &angles){
	glm::mat4 rotMat(1.0f);
	rotMat = glm::rotate(rotMat, angles.x, glm::vec3(1.0f,0.0f,0.0f));
	rotMat = glm::rotate(rotMat, angles.y, glm::vec3(0.0f,1.0f,0.0f));
	rotMat = glm::rotate(rotMat, angles.z, glm::vec3(0.0f,0.0f,1.0f));
	m_position = glm::vec3(rotMat * glm::vec4(m_position,1.0f));
	computeViewMatrix();

}

void Camera::advance(float offset){
	m_position += (m_forwardVector* - offset);
	computeViewMatrix();
}

void Camera::strafe(float offset){
	m_position += (m_rightVector * offset);
	computeViewMatrix();
}

void Camera::ascend(float offset){
	m_position += (m_upVector * offset);
	computeViewMatrix();
}

void Camera::yaw(float angle){
	m_forwardVector = glm::normalize(m_forwardVector * cos(degToRad(angle)) - m_rightVector * sin(degToRad(angle)));
	m_rightVector = glm::cross(m_forwardVector,m_upVector);
	computeViewMatrix();
}

void Camera::pitch(float angle){
	m_forwardVector = glm::normalize(m_forwardVector * cos(degToRad(angle)) + m_upVector * sin(degToRad(angle)));
	m_upVector = glm::cross(m_forwardVector, m_rightVector);
	computeViewMatrix();
}

void Camera::roll(float angle){
	m_rightVector = glm::normalize(m_rightVector * cos(degToRad(angle)) + m_upVector * sin(degToRad(angle)));
	m_upVector = glm::cross(m_forwardVector, m_rightVector);
	computeViewMatrix();
}

void Camera::setPosition(const glm::vec3 &position){
	m_position = position;
	computeViewMatrix();
}

void Camera::setUpVector(const glm::vec3 &upVector){
	m_upVector = glm::normalize(upVector);
	m_rightVector = glm::cross(m_upVector, m_forwardVector);
	computeViewMatrix();
}

void Camera::setForwardVector(const glm::vec3 &forwardVector){
	m_forwardVector = glm::normalize(forwardVector);
	m_rightVector = glm::cross(m_upVector, m_forwardVector);
	computeViewMatrix();
}

void Camera::setCameraAxis(const glm::vec3 &position, const glm::vec3 &upVector, const glm::vec3 &forwardVector){
	m_upVector = glm::normalize(upVector);
	m_position = position;
	m_forwardVector = glm::normalize(forwardVector);
	m_rightVector = glm::cross(m_upVector,m_forwardVector);
	computeViewMatrix();
}


void Camera::computeViewMatrix(){
	if(m_targeting){
		m_viewMatrix = glm::lookAt(m_position, m_target, m_upVector);
	}else
		m_viewMatrix = glm::lookAt(m_position,m_position+m_forwardVector,m_upVector);

}

void Camera::setPerspectiveParam(float FOV, float aspect, float nearZ, float farZ){
	m_FOV = FOV; 
	m_aspectRatio = aspect;
	m_nearClip = nearZ;
	m_farClip = farZ;
	computePerspectiveMatrix();
}

void Camera::setFOV(float FOV){
	m_FOV = FOV;
	computePerspectiveMatrix();
}

void Camera::setAspectRatio(float aspect){
	m_aspectRatio = aspect;
	computePerspectiveMatrix();
}

void Camera::setZClip(float near, float far){
	m_nearClip = near; 
	m_farClip = far;
	computePerspectiveMatrix();
}

void Camera::computePerspectiveMatrix(){
	m_perspectiveMatrix = glm::perspective(m_FOV,m_aspectRatio, m_nearClip, m_farClip);
}


void Camera::setTarget(const glm::vec3 &target){
	m_targeting = true;
	m_target = target;
	computeViewMatrix();
}

void Camera::deactivateTargetting(){
	m_targeting = false;
	computeViewMatrix();
}


void Camera::adaptFOV(float objectRadius, float distance){
	//	distance+=objectRadius;
	float FOV = asin((objectRadius)/(distance));
	setFOV(FOV*180.0f);
}

glm::vec3 Camera::getCameraPosition() const{
	return m_position;
}

glm::vec2 Camera::getClipPlanes() const{
	return glm::vec2(m_nearClip, m_farClip);
}

glm::mat4 Camera::getViewMatrix() const{
	return m_viewMatrix;
}

glm::mat4 Camera::getPMatrix() const{
	return m_perspectiveMatrix;
}


