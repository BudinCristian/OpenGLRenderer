#include "PointLight.h"

PointLight::PointLight()
{
	position = glm::vec3(0.f, 0.f, 0.f);
	constant = 1.f;
	linear = 0.f;
	exponent = 0.f;
}

PointLight::PointLight(GLfloat red, GLfloat green, GLfloat blue,
	GLfloat aIntensity, GLfloat dIntensity,
	GLfloat xPos, GLfloat yPos, GLfloat zPos, 
	GLfloat con, GLfloat lin, GLfloat exp) : Light(red, green, blue, aIntensity, dIntensity)
{
	position = glm::vec3(xPos, yPos, zPos);
	constant = con;
	linear = lin;
	exponent = exp;
}



void PointLight::UseLight(GLfloat ambientIntensityLocation, GLfloat ambientColorLocation, 
	GLfloat diffuseIntensityLocation, GLfloat positionLocation,
	GLfloat constantLocation, GLfloat linearLocation, GLfloat exponentLocation)
{
	glUniform3f(ambientColorLocation, color.x, color.y, color.z);
	glUniform1f(ambientIntensityLocation, ambientIntensity);
	glUniform1f(diffuseIntensityLocation, diffuseIntensity);

	glUniform3f(positionLocation, position.x, position.y, position.z);
	glUniform1f(constantLocation, constant);
	glUniform1f(linearLocation, linearLocation);
	glUniform1f(exponentLocation, exponent);
}
