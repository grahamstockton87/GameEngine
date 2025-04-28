#pragma once

#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include "Mesh.h"

class Camera {
public:
	Camera();
	Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed, GLfloat startRollSpeed, GLfloat startFov);
	Camera& operator=(const Camera& other) = default;


	void keyControl(bool* keys, GLfloat deltaTime);
	void mouseControl(GLfloat xChange, GLfloat yChange, GLfloat yScrollChange, GLfloat deltaTime); 
	void updatePhysics(GLfloat deltaTime);
	void boxCollision(Mesh::BoundingBox box);
	void setPostion(glm::vec3 pos);

	glm::vec3 getCameraPostion();
	glm::vec3 getCameraDirection();

	glm::mat4 calculateViewMatrix();

	GLfloat getFov();
	void setFov(GLfloat Infov);



	~Camera();
private:
	glm::vec3 position;
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

	bool isGrounded;
	GLfloat verticalVelocity = 0.0f;
	const GLfloat gravity = -50.0f; // Or tweak to your scale
	const GLfloat terminalVelocity = 50.0f;

	glm::vec3 previousPosition;

	void update();

	bool intersects(Mesh::BoundingBox box);

};

