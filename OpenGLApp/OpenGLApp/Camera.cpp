#include "Camera.h"


Camera::Camera()
{
}


Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed)
{
	position = startPosition;
	worldUp = startUp;

	yaw = startYaw;
	pitch = startPitch;

	front = glm::vec3(0.f, 0.f, -1.f);

	movementSpeed = startMoveSpeed;
	turnSpeed = startTurnSpeed;

	update();
}

void Camera::keyControl(bool* keys, GLfloat deltaTime)
{
	if(!isAnimating){
		if (keys[GLFW_KEY_M])
		{
			startAnimation();
		}

		if (keys[GLFW_KEY_W])
		{
			position += front * movementSpeed * deltaTime;
		}

		if (keys[GLFW_KEY_S])
		{
			position -= front * movementSpeed * deltaTime;
		}

		if (keys[GLFW_KEY_A])
		{
			position -= right * movementSpeed * deltaTime;
		}

		if (keys[GLFW_KEY_D])
		{
			position += right * movementSpeed * deltaTime;
		}
	}
	else
	{
		updateAnimation(deltaTime);
	}
}

void Camera::mouseControl(GLfloat xChange, GLfloat yChange)
{
	if(!isAnimating)
			{ 
	xChange *= turnSpeed;
	yChange *= turnSpeed;

	yaw += xChange;
	pitch += yChange;

	if (pitch > 89.f)
	{
		pitch = 89.f;
	}

	if (pitch < -89.f)
	{
		pitch = -89.f;
	}

	update();
	}
}

glm::mat4 Camera::calculateViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

void Camera::startAnimation()
{
	if (!isAnimating)
	{
		isAnimating = true;
		animationStartPos = position;
		animationEndPos = glm::vec3(0.f, 20.f, -12.f);
		animationStartTime = glfwGetTime();
		animationDuration = 15.f;

		cameraLookAt = glm::vec3(0.f, 7.f, 0.f);

		controlPoints.clear();
		controlPoints.push_back(animationStartPos);
		controlPoints.push_back(glm::vec3(30.f, 20.f, 0.f));
		controlPoints.push_back(glm::vec3(0.f, 20.f, 30.f));
		controlPoints.push_back(glm::vec3(-12.f, 20.f, 0.f));

		controlPoints.push_back(animationEndPos);  // endPos
		
		glm::vec3 initialDirection = glm::normalize(controlPoints[1] - animationStartPos);
		yaw = glm::degrees(atan2(initialDirection.z, initialDirection.x));
		pitch = glm::degrees(asin(initialDirection.y));

	}
}
void Camera::updateAnimation(GLfloat deltaTime)
{
	if (isAnimating)
	{
		float elapsedTime = glfwGetTime() - animationStartTime;
		if (elapsedTime < animationDuration)
		{
			float t = elapsedTime / animationDuration;
			position = calculateBezierPoint(t);

			// Calculate dynamic target position based on the current camera position
			glm::vec3 dynamicTarget = position + (cameraLookAt - position) * 0.5f;

			// Update orientation (yaw and pitch) based on the dynamic target position
			glm::vec3 direction = glm::normalize(dynamicTarget - position);
			yaw = glm::degrees(atan2(direction.z, direction.x));
			pitch = glm::degrees(asin(direction.y));

			// Ensure pitch is within a reasonable range
			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;
		}
		else
		{
			// Animation finished
			position = animationEndPos;
			isAnimating = false;
		}
		update(); // Update orientation vectors
	}
}

glm::vec3 Camera::calculateBezierPoint(float t)
{
	int n = controlPoints.size() - 1;
	std::vector<glm::vec3> p = controlPoints;

	// Apply cubic easing to the parameter t
	t = t * t * (3.0f - 2.0f * t);

	for (int k = 1; k <= n; ++k)
	{
		for (int i = 0; i <= n - k; ++i)
		{
			p[i] = (1 - t) * p[i] + t * p[i + 1];
		}
	}

	return p[0];
}



void Camera::update()
{
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}
