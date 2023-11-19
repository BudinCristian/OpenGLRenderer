#define  STB_IMAGE_IMPLEMENTATION

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW//glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Texture.h"

#include "Mesh.h"
#include "Shader.h"
#include "Window.h"
#include "Camera.h"
#include "Texture.h"
#include "DirectionalLight.h"
#include "Material.h"
#include "PointLight.h"

#include "CommonValues.h"


Window mainWindow;
const float toRadians = glm::pi<float>() / 180.f;

std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture brickTexture;
Texture dirtTexture;

DirectionalLight mainLight;

PointLight pointLights[MAX_POINT_LIGHTS];

Material shinyMaterial;
Material matteMaterial;

GLfloat deltaTime = 0.f;
GLfloat lastTime = 0.f;

// Vertex Shader
static const char* vShader = "shaders/shader.vert";

// Fragment Shader
static const char* fShader = "shaders/shader.frag";

void calcAverageNormals(unsigned int* indices, unsigned int indicesCount, GLfloat* vertices, unsigned int verticesCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indicesCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;

		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset;
		in1 += normalOffset;
		in2 += normalOffset;

		vertices[in0] += normal.x;
		vertices[in0+1] += normal.y;
		vertices[in0+2] += normal.z;

		vertices[in1] += normal.x;
		vertices[in1 + 1] += normal.y;
		vertices[in1 + 2] += normal.z;

		vertices[in2] += normal.x;
		vertices[in2 + 1] += normal.y;
		vertices[in2 + 2] += normal.z;


	}

	for (size_t i = 0; i < verticesCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x;
		vertices[nOffset+1] = vec.y;
		vertices[nOffset+2] = vec.z;
	}
}


void CreateObjects()
{
	unsigned int indices[] = {
		0,3,1,
		1,3,2,
		2,3,0,
		0,1,2
	};

	GLfloat vertices[] = {
		//	x	  y	     z			u    v			 nx	 ny   nz
			-1.f, -1.f, -0.6f,    0.f, 0.f,			 0.f, 0.f, 0.f,
			0.f, -1.f, 1.f,		  0.5, 0.f,			 0.f, 0.f, 0.f,
			1.f, -1.f, -0.6f,	  1.f, 0.f,			 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f,        0.5f, 1.f,		 0.f, 0.f, 0.f
	};

	calcAverageNormals(indices, 12, vertices, 32, 8, 5);

	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh* obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

int main()
{
	mainWindow = Window(1366, 768); //1280:1024, 1024:768
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), -90.f, 0.f, 5.f, 0.05f);

	brickTexture = Texture((char*)("textures/brick.png"));
	brickTexture.LoadTexture();

	dirtTexture = Texture((char*)("textures/dirt.png"));
	dirtTexture.LoadTexture();

	shinyMaterial = Material(1.f, 32);
	matteMaterial = Material(0.3f, 4);

	mainLight = DirectionalLight(1.f, 1.f, 1.f,
								0.1f, 0.3f,
								0.f, 0.0f, -1.f);

	unsigned int pointLightCount = 0;

	pointLights[pointLightCount++] = PointLight(
		0.f, 0.f, 1.f,
		0.1f, 0.4f,
		4.0f, 0.f, 0.f,
		0.3f, 0.2f, 0.1f
	);

	pointLights[pointLightCount++] = PointLight(
		0.f, 1.f, 0.f,
		0.1f, 1.f,
		-4.0f, 2.f, 0.f,
		0.3f, 0.1f, 0.1f
	);
	
	glm::mat4 projection = glm::perspective(45.f, (GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(), 0.1f, 100.f);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;


	// Loop until window closed
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		// Get + Handle user input events
		glfwPollEvents();

		camera.keyControl(mainWindow.getKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Clear window
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);


		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.GetCameraPosition().x, camera.GetCameraPosition().y, camera.GetCameraPosition().z);


		glm::mat4 model(1.f);
		model = glm::translate(model, glm::vec3(0.f, 0.0f, -2.5f));
		// model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		brickTexture.UseTexture();
		shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[0]->RenderMesh();

		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(0.f, 4.0f, -2.5f));
		// model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		dirtTexture.UseTexture();
		matteMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[1]->RenderMesh();

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}