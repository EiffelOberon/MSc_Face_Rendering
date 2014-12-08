#ifndef CAMERA_H
#define CAMERA_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/integer.hpp"
#include "utils.h"



class Camera{

private: 
	glm::vec3 m_position;	// Where the camera is.
	glm::vec3 m_upVector;	// View Up Vector.
	glm::vec3 m_forwardVector;	// View Normal Plane, aka direction.
	glm::vec3 m_rightVector;

	bool m_targeting;
	glm::vec3 m_target;

	float m_FOV;
	float m_nearClip;
	float m_farClip;
	float m_aspectRatio;

	glm::mat4 m_viewMatrix;
	glm::mat4 m_perspectiveMatrix;

	void computeViewMatrix();
	void computePerspectiveMatrix();

public:
	Camera();
	Camera(glm::vec3 position, glm::vec3 forwardVector);
	Camera(glm::vec3 position, glm::vec3 upVector, glm::vec3 forwardVector);
	Camera(glm::vec3 position, glm::vec3 upVector, glm::vec3 forwardVector, float FOV, float aspect, float nearZ, float farZ);


	// Camera movements
	void translateCamera(const glm::vec3 &displacement);
	void rotateCameraPosition(const glm::vec3 &angles);
	void advance(float offset);
	void strafe(float offset);
	void ascend(float offset);
	void pitch(float angle);
	void yaw(float angle);
	void roll(float angle);

	// Perspective related
	void setPerspectiveParam(float FOV, float aspect, float nearZ, float farZ);
	void setFOV(float FOV);
	void setAspectRatio(float aspect);
	void setZClip(float near, float far);

	// Axis / position related [Not 
	void setCameraAxis(const glm::vec3 &position, const glm::vec3 &upVector, const glm::vec3 &forwardVector);
	void setPosition(const glm::vec3 &position);
	void setUpVector(const glm::vec3 &upVector);
	void setForwardVector(const glm::vec3 &forwardVector);

	void setTarget(const glm::vec3 &target);
	void deactivateTargetting();

	void adaptFOV(float objectRadius, float distance);


	glm::vec3 getCameraPosition() const;
	glm::vec2 getClipPlanes() const;
	glm::mat4 getViewMatrix() const;
	glm::mat4 getPMatrix() const;


};

#endif