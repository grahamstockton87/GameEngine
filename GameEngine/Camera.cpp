#include "Camera.h"
#include <iostream>
#include "Mesh.h"
#include "BoundingBox.h"

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

	// Sprinting logic with cooldown
	if (keys[GLFW_KEY_LEFT_SHIFT] && canSprint)
	{
		if (sprintTimer < maxSprintDuration)
		{
			isSprinting = true;
			sprintTimer += deltaTime;
			velocity *= 2.0f;  // Sprint speed
		}
		else
		{
			// Sprint exhausted, start cooldown
			isSprinting = false;
			canSprint = false;
			sprintCooldownTimer = 0.0f;
		}
	}
	else
	{
		isSprinting = false;
		if (!canSprint)
		{
			// Cooldown active
			sprintCooldownTimer += deltaTime;
			if (sprintCooldownTimer >= sprintCooldownDuration)
			{
				// Cooldown finished, reset timers
				canSprint = true;
				sprintTimer = 0.0f;
			}
		}
		else
		{
			// If not sprinting, recover sprint time if partially used
			if (sprintTimer > 0.0f)
			{
				sprintTimer = glm::max(0.0f, sprintTimer - deltaTime * 2.0f);
			}
		}
	}

	// Standard movement
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

	if (keys[GLFW_KEY_SPACE] && isGrounded) {
		verticalVelocity = 10.0f;
		isGrounded = false;
	}

	if (keys[GLFW_KEY_LEFT_CONTROL]) {
		position.y -= velocity;
	}
}


void Camera::mouseControl(GLfloat xChange, GLfloat yChange, GLfloat yScrollChange, bool leftClicked, GLfloat deltaTime)
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
	const float epsilon = 0.01f;

	if (!isGrounded) {
		verticalVelocity += gravity * deltaTime;
		position.y += verticalVelocity * deltaTime + 0.5f * gravity * deltaTime * deltaTime;

		// Floor clamp (failsafe if we fall through)
		if (position.y < groundLevel + radiusY - epsilon) {
			isGrounded = true;
			verticalVelocity = 0.0f;
			position.y = groundLevel + radiusY;
		}
	}
	else {
		// Player is grounded — keep them pinned
		verticalVelocity = 0.0f;
		position.y = groundLevel + radiusY;
	}
}

bool Camera::boxCollision(const BoundingBox& box, GLfloat deltaTime, float& outGroundLevel)
{
	const float epsilon = 0.5f;

	if (!intersects(box)) {
		return false;
	}

	float feet = position.y - radiusY + 0.1;
	glm::vec3 delta = position - previousPosition;

	bool feetOnTop = (feet >= box.max.y - epsilon && feet <= box.max.y + epsilon);
	bool xInside = position.x >= box.min.x && position.x <= box.max.x;
	bool zInside = position.z >= box.min.z && position.z <= box.max.z;

	bool landed = feetOnTop || (xInside && zInside);
	if (landed) {
		outGroundLevel = box.max.y;
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
bool Camera::intersects(const BoundingBox& box) const
{
	float feet = position.y - radiusY + 0.01;
	bool xInside = (position.x >= box.min.x && position.x <= box.max.x);
	bool yInside = (feet >= box.min.y && feet <= box.max.y);
	bool zInside = (position.z >= box.min.z && position.z <= box.max.z);

	return xInside && yInside && zInside;
}

BoundingBox Camera::GetBoundingBox()
{
	glm::vec3 minBound = position - glm::vec3(radiusX, radiusY, radiusZ);
	glm::vec3 maxBound = position + glm::vec3(radiusX, radiusY, radiusZ);
	return BoundingBox{ minBound, maxBound };
}


Camera::~Camera()
{
}
