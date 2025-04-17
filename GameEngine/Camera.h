#pragma once

#include <glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera {
public:
	Camera();
	Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed, GLfloat startRollSpeed, GLfloat startFov);


	void keyControl(bool* keys, GLfloat deltaTime);
	void mouseControl(GLfloat xChange, GLfloat yChange, GLfloat yScrollChange, GLfloat deltaTime);

	glm::vec3 getCameraPostion();

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

	void update();

};

