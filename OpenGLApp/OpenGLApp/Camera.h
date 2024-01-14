#pragma once

#include <Gl\glew.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <GLFW/glfw3.h>
#include <vector>

class Camera
{
public:
	Camera();

	Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed);

	void keyControl(bool* keys, GLfloat deltaTime);
	void mouseControl(GLfloat xChange, GLfloat yChange);

	glm::vec3 GetCameraPosition() { return position; }
	glm::vec3 GetCameraDirection() { return glm::normalize(front); }
	glm::mat4 calculateViewMatrix();

	void startAnimation();
	void updateAnimation(GLfloat deltaTime);

	~Camera() = default;

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	GLfloat yaw;
	GLfloat pitch;

	GLfloat movementSpeed;
	GLfloat turnSpeed;

	bool isAnimating;
	glm::vec3 animationStartPos;
	glm::vec3 animationEndPos;
	glm::vec3 cameraLookAt;
	float animationStartTime;
	float animationDuration;

	// Additional variables for Bezier spline
	std::vector<glm::vec3> controlPoints;

	void update();
	glm::vec3 calculateBezierPoint(float t);
};

