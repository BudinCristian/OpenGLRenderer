#pragma once

#include <GL\glew.h>
#include <glm\glm.hpp>

#include "ShadowMap.h"

class Light
{
public:
	Light();

	Light(
		GLfloat shadowWidth, GLfloat shadowHeight,
		GLfloat red, GLfloat green, GLfloat blue,
		GLfloat aIntensity, GLfloat dIntensity
	);

	~Light() = default;

	ShadowMap* GetShadowMap() { return shadowMap; }
protected:
	glm::vec3 color;
	GLfloat ambientIntensity;
	GLfloat diffuseIntensity;

	glm::mat4 lightProj;

	ShadowMap* shadowMap;
};

