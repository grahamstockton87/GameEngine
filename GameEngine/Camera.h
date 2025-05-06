#pragma once

#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include "Mesh.h"
#include "Triangle.h"

class Camera {
public:
	Camera();
	Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed, GLfloat startRollSpeed, GLfloat startFov);
	Camera& operator=(const Camera& other) = default;


	void keyControl(bool* keys, GLfloat deltaTime);
	void mouseControl(GLfloat xChange, GLfloat yChange, GLfloat yScrollChange, GLfloat deltaTime); 
	void updatePhysics(GLfloat deltaTime);
	bool boxCollision(const Mesh::BoundingBox& box, GLfloat deltaTime, float& outGroundLevel);
	void setPostion(glm::vec3 pos);

	glm::vec3 getCameraPostion();
	glm::vec3 getCameraDirection();
	bool isOnGround() const;

	glm::mat4 calculateViewMatrix();

	GLfloat getFov();
	void setFov(GLfloat Infov);

	bool isGrounded;
	glm::vec3 position;
	glm::vec3 previousPosition;
	float groundLevel = 0;

	GLfloat verticalVelocity = 0.0f;
	const GLfloat gravity = -20.0f; // Or tweak to your scale
	const GLfloat terminalVelocity = 50.0f;

	const float height = 2.0f;
	const float radiusX = 0.1f;    // Half-width (side-to-side)
	const float radiusZ = 0.1f;    // Half-depth (front-to-back)
	const float radiusY = 2.0f;

	~Camera();
private:
	
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	GLfloat yaw;
	GLfloat pitch;
	GLfloat roll;

	GLfloat moveSpeed;
	GLfloat turnSpeed;
	GLfloat rollSpeed;

	GLfloat fov;

	


	bool prevJumpPressed = false;

	

	void update();

	bool intersects(const Mesh::BoundingBox& box) const;

};

