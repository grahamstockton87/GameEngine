#include "Camera.h"

Camera::Camera()
{
}

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed, GLfloat startRollSpeed)
{
	position = startPosition;
	worldUp = startUp;
	yaw = startYaw;
	pitch = startPitch;
	roll = 0;
	front = glm::vec3(0.0f, 0.0f, -1.0f);

	moveSpeed = startMoveSpeed;
	turnSpeed = startTurnSpeed;
	rollSpeed = startRollSpeed;

	update();
}

void Camera::keyControl(bool* keys, GLfloat deltaTime)
{
	GLfloat velocity = moveSpeed * deltaTime;
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]) {
		position += front * velocity;
	}
	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]) {
		position -= front * velocity;
	}
	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) {
		position += right * velocity;
	}
	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) {
		position -= right * velocity;
	}
	if (keys[GLFW_KEY_E]) {
		roll += velocity + rollSpeed;
		update();
	}
	if (keys[GLFW_KEY_Q]) {
		roll -= velocity + rollSpeed;
		update();
	}
	if (keys[GLFW_KEY_SPACE]) {
		position += up * velocity;
	}
	if (keys[GLFW_KEY_LEFT_CONTROL]) {
		position -= up * velocity;
	}

}

void Camera::mouseControl(GLfloat xChange, GLfloat yChange, GLfloat deltaTime)
{
	GLfloat velocity = turnSpeed * deltaTime;
	xChange *= velocity;
	yChange *= velocity;

	yaw += xChange;
	pitch += yChange;

	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}
	// frotn and right could have chanegd so update 
	update();

}

glm::mat4 Camera::calculateViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}


void Camera::update()
{
	// Compute the front vector (camera direction)
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);

	// Compute the right and up vectors
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));

	// Roll effect: Rotate right and up around the front axis by the roll angle
	glm::mat4 rollMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(roll), front);

	// Apply the roll to the right and up vectors
	right = glm::normalize(glm::vec3(rollMatrix * glm::vec4(right, 0.0f)));
	up = glm::normalize(glm::vec3(rollMatrix * glm::vec4(up, 0.0f)));
}


Camera::~Camera()
{
}
