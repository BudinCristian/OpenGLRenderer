#pragma once

#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>

#include <GL/glew.h>

#include "DirectionalLight.h"
#include "PointLight.h"
#include "CommonValues.h"

class Shader
{
public:
	Shader();
	
	void CreateFromString(const char* vertexCode, const char* fragmentCode);
	void CreateFromFiles(const char* vertexLocation, const char* fragmentLocation);

	std::string ReadFile(const char* fileLocation);

	GLuint GetProjectionLocation();
	GLuint GetModelLocation();
	GLuint GetViewLocation();
	GLuint GetAmbientIntensityLocation()  { return uniformDirectionalLight.uniformAmbientIntensity; }
	GLuint GetAmbientColorLocation()  { return uniformDirectionalLight.uniformColor; }
	GLuint GetDiffuseIntensityLocation() const { return uniformDirectionalLight.uniformDiffuseIntensity; }
	GLuint GetDirectionLocation() const { return uniformDirectionalLight.uniformDirection; }
	GLuint GetSpecularIntensityLocation() { return uniformSpecularIntensity; }
	GLuint GetShininessLocation() { return uniformShininess; }
	GLuint GetEyePositionLocation() { return uniformEyePosition; }

	void SetDirectionalLight(DirectionalLight* dLight);
	void SetPointLights(PointLight* pLight, unsigned int lightCount);

	void UseShader();
	void ClearShader();

	~Shader();

private:
	int pointLightCount;

	GLuint shaderID, uniformProjection, uniformModel, uniformView, uniformEyePosition,
		uniformSpecularIntensity, uniformShininess;

	struct
	{
		GLuint uniformColor;
		GLuint uniformAmbientIntensity;
		GLuint uniformDiffuseIntensity;

		GLuint uniformDirection;
	} uniformDirectionalLight;

	GLuint uniformPointLightCount;

	struct
	{
		GLuint uniformColor;
		GLuint uniformAmbientIntensity;
		GLuint uniformDiffuseIntensity;

		GLuint uniformPosition;
		GLuint uniformConstant;
		GLuint uniformLinear;
		GLuint uniformExponent;
	} uniformPointLights[MAX_POINT_LIGHTS];

	void CompileShader(const char* vertexCode, const char* fragmentCode);
	void AddShader(GLuint program, const char* shaderCode, GLenum shaderType);
};

