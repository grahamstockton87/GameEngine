#include "Camera.h"
#include <iostream>
#include "Mesh.h"
Camera::Camera()
{
}

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed, GLfloat startRollSpeed, GLfloat startFov)
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

	fov = startFov;

	isGrounded = false;

	update();
}

void Camera::keyControl(bool* keys, GLfloat deltaTime)
{
	previousPosition = position;

	GLfloat velocity = moveSpeed * deltaTime;
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]) {
		glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
		position += flatFront * velocity;

	}
	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]) {
		glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
		position -= flatFront * velocity;
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
		if (isGrounded) {
			verticalVelocity = 20.0f;
			isGrounded = false;
		}
	}
	if (keys[GLFW_KEY_LEFT_CONTROL]) {
		position.y -= velocity;
	}

}

void Camera::mouseControl(GLfloat xChange, GLfloat yChange, GLfloat yScrollChange, GLfloat deltaTime)
{
	GLfloat velocity = turnSpeed * deltaTime;
	xChange *= velocity;
	yChange *= velocity;

	yaw += xChange;
	pitch += yChange;

	// Only update fov if yScrollChange is not zero
	if (yScrollChange != 0.0f) {
		fov += yScrollChange * deltaTime * 10.0f;
		fov = glm::clamp(fov, 30.0f, 90.0f);  // Clamp the fov to stay within the 30-90 range
	}

	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}
	// frotn and right could have chanegd so update 
	update();

}

void Camera::updatePhysics(GLfloat deltaTime)
{
	float height = 2.0f;
	float floor = 0.0f;

	//std::cout << "Postion y " << position.y << std::endl;
	//std::cout << isGrounded;
	
	if (position.y > floor + height || !isGrounded) {
		isGrounded = false;
		verticalVelocity += gravity * deltaTime;
		position.y += verticalVelocity * deltaTime + 0.5f * gravity * deltaTime * deltaTime;

	}
	if (position.y < floor + height){
		isGrounded = true;
		position.y = height;
	}
}

void Camera::boxCollision(Mesh::BoundingBox box)
{
	if (!intersects(box)) return;

	glm::vec3 delta = position - previousPosition;

	// Resolve Y-axis (vertical) collision
	if (previousPosition.y >= box.max.y && position.y < box.max.y) {
		// Landing on top of the box
		isGrounded = true;
		verticalVelocity = 0.0f;
		position.y = box.max.y;
	}
	else {
		// Not a top collision — resolve sides
		if (delta.x != 0 && previousPosition.x <= box.min.x && position.x > box.min.x) {
			position.x = previousPosition.x;
		}
		if (delta.x != 0 && previousPosition.x >= box.max.x && position.x < box.max.x) {
			position.x = previousPosition.x;
		}

		if (delta.z != 0 && previousPosition.z <= box.min.z && position.z > box.min.z) {
			position.z = previousPosition.z;
		}
		if (delta.z != 0 && previousPosition.z >= box.max.z && position.z < box.max.z) {
			position.z = previousPosition.z;
		}

		// Optional: handle vertical head bump
		if (delta.y > 0 && previousPosition.y <= box.min.y && position.y > box.min.y) {
			position.y = previousPosition.y;
			verticalVelocity = 0.0f; // stop rising if bumping
		}
	}
}



void Camera::setPostion(glm::vec3 pos)
{
	position = pos;
}

glm::vec3 Camera::getCameraPostion()
{
	return position;
}

glm::vec3 Camera::getCameraDirection()
{
	return glm::normalize(front);
}

glm::mat4 Camera::calculateViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

GLfloat Camera::getFov()
{
	return fov;
}

void Camera::setFov(GLfloat Infov)
{
	fov = Infov;
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
bool Camera::intersects(Mesh::BoundingBox box)
{
	const float radius = 0.5f; // Adjust based on how wide your camera/player is

	return (position.x >= box.min.x &&
		position.x <= box.max.x &&
		position.y >= box.min.y &&
		position.y <= box.max.y &&
		position.z >= box.min.z &&
		position.z <= box.max.z);
}



Camera::~Camera()
{
}
