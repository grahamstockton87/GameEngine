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

	// Jump edge trigger
	if (keys[GLFW_KEY_SPACE] && isGrounded) {
		verticalVelocity = 20.0f;
		position.y += 0.1f;
		isGrounded = false;
	}
	else if (keys[GLFW_KEY_SPACE] && !isGrounded){
		std::cout << 0;
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
	if (!isGrounded) {
		verticalVelocity += gravity * deltaTime;
		position.y += verticalVelocity * deltaTime + 0.5f * gravity * deltaTime * deltaTime;
		if (position.y < groundLevel) {
			isGrounded = true;
			verticalVelocity = 0.0f;
			position.y = radiusY + groundLevel;
		}
	}
	else {
		verticalVelocity = 0.0f;
		position.y = radiusY + groundLevel;
	}
	//std::cout << "ground level " << groundLevel << std::endl;
	
}


bool Camera::boxCollision(const Mesh::BoundingBox& box, GLfloat deltaTime, float& outGroundLevel)
{
	const float epsilon = 0.5f;

	if (!intersects(box)) return false;

	float feet = position.y - radiusY;
	glm::vec3 delta = position - previousPosition;

	bool feetOnTop = (feet >= box.max.y - epsilon && feet <= box.max.y + epsilon);
	bool xInside = position.x >= box.min.x && position.x <= box.max.x;
	bool zInside = position.z >= box.min.z && position.z <= box.max.z;

	bool landed = feetOnTop || (xInside && zInside);
	if (landed) {
		outGroundLevel = box.max.y;
	}

	// Horizontal pushback logic remains
	const float pushback = 0.05f;
	if (delta.x > 0 && previousPosition.x + pushback <= box.min.x && position.x + pushback > box.min.x) {
		position.x = previousPosition.x;
	}
	else if (delta.x < 0 && previousPosition.x - pushback >= box.max.x && position.x - pushback < box.max.x) {
		position.x = previousPosition.x;
	}
	if (delta.z > 0 && previousPosition.z + pushback <= box.min.z && position.z + pushback > box.min.z) {
		position.z = previousPosition.z;
	}
	else if (delta.z < 0 && previousPosition.z - pushback >= box.max.z && position.z - pushback < box.max.z) {
		position.z = previousPosition.z;
	}

	return landed;
}







bool Camera::isOnGround() const {
	return isGrounded;
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
bool Camera::intersects(const Mesh::BoundingBox& box) const
{
	float feet = position.y - radiusY;
	bool xInside = (position.x >= box.min.x && position.x <= box.max.x);
	bool yInside = (feet >= box.min.y && feet <= box.max.y);
	bool zInside = (position.z >= box.min.z && position.z <= box.max.z);

	std::cout << "Checking intersect: "
		<< "pos=(" << position.x << ", " << position.y << ", " << position.z << "), "
		<< "feet=" << feet << ", "
		<< "boxY=[" << box.min.y << "," << box.max.y << "], "
		<< "xInside=" << xInside << ", yInside=" << yInside << ", zInside=" << zInside << std::endl;

	return xInside && yInside && zInside;
}








Camera::~Camera()
{
}
