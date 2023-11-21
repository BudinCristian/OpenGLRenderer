#define  STB_IMAGE_IMPLEMENTATION

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

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
#include "SpotLight.h"

#include "CommonValues.h"

#include "assimp/Importer.hpp"

#include "Model.h"
#include "Skybox.h"


const float toRadians = glm::pi<float>() / 180.f;

GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
uniformSpecularIntensity = 0, uniformShininess = 0,
uniformOmniLightPos = 0, uniformFarPlane = 0;


Window mainWindow;

std::vector<Mesh*> meshList;

std::vector<Shader> shaderList;
Shader directionalShadowShader;
Shader omniShadowShader;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;

DirectionalLight mainLight;

PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

Skybox skybox;

unsigned int pointLightCount = 0;
unsigned int spotLightCount = 0;

Material shinyMaterial;
Material matteMaterial;

Model xwing;
Model blackhawk;

GLfloat deltaTime = 0.f;
GLfloat lastTime = 0.f;

GLfloat blackhawkAngle = 0.f;

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
		vertices[in0 + 1] += normal.y;
		vertices[in0 + 2] += normal.z;

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
		vertices[nOffset + 1] = vec.y;
		vertices[nOffset + 2] = vec.z;
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

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3,
	};

	GLfloat floorVertices[] = {
		-20.f, 0.f, -20.f,  0.f, 0.f,		0.f, -1.f, 0.f,
		20.f, 0.f, -20.f,	10.f, 0.f,		0.f, -1.f, 0.f,
		-20.f, 0.f, 20.f,   0.f, 10.f,		0.f, -1.f, 0.f,
		20.f, 0.f, 20.f,	10.f, 10.f,		0.f, -1.f, 0.f
	};

	calcAverageNormals(indices, 12, vertices, 32, 8, 5);

	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh* obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh* obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);

}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);

	directionalShadowShader = Shader();
	directionalShadowShader.CreateFromFiles("shaders/directional_shadow_map.vert", "shaders/directional_shadow_map.frag");
	omniShadowShader.CreateFromFiles("shaders/omni_shadow_map.vert", "shaders/omni_shadow_map.geom", "shaders/omni_shadow_map.frag");
}

void RenderScene()
{
	glm::mat4 model(1.f);
	model = glm::translate(model, glm::vec3(0.f, 0.0f, -2.5f));
	//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	brickTexture.UseTexture();
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();

	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(0.f, 4.0f, -2.5f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dirtTexture.UseTexture();
	matteMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[1]->RenderMesh();

	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(0.f, -2.0f, 0.f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dirtTexture.UseTexture();
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[2]->RenderMesh();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-7.0f, 0.0f, 10.0f));
	model = glm::scale(model, glm::vec3(0.006f, 0.006f, 0.006f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	xwing.RenderModel();

	blackhawkAngle += 0.1;
	if (blackhawkAngle > 360.f)
	{
		blackhawkAngle = 0.1f;
	}

	model = glm::mat4(1.0f);
	model = glm::rotate(model, -blackhawkAngle * toRadians, glm::vec3(0.f, 1.f, 0.f));
	model = glm::translate(model, glm::vec3(-8.0f, 2.0f, 0.0f));
	model = glm::rotate(model, -20.f * toRadians, glm::vec3(0.f, 0.f, 1.f));
	model = glm::rotate(model, -90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	blackhawk.RenderModel();
}

void DirectionalShadowMapPass(DirectionalLight* light)
{
	directionalShadowShader.UseShader();

	glViewport(0, 0, light->GetShadowMap()->GetShadowWidth(), light->GetShadowMap()->GetShadowHeight());

	light->GetShadowMap()->Write();
	glClear(GL_DEPTH_BUFFER_BIT);

	uniformModel = directionalShadowShader.GetModelLocation();

	glm::mat4 lightTransform = light->CalculateLightTransform();
	directionalShadowShader.SetDirectionalLightTransform(&lightTransform);

	directionalShadowShader.Validate();

	RenderScene();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OmniShadowMapPass(PointLight* light)
{
	omniShadowShader.UseShader();

	glViewport(0, 0, light->GetShadowMap()->GetShadowWidth(), light->GetShadowMap()->GetShadowHeight());

	light->GetShadowMap()->Write();
	glClear(GL_DEPTH_BUFFER_BIT);

	uniformModel = omniShadowShader.GetModelLocation();
	uniformOmniLightPos = omniShadowShader.GetOmniLightPosLocation();
	uniformFarPlane = omniShadowShader.GetFarPlaneLocation();

	glUniform3f(uniformOmniLightPos, light->GetPosition().x, light->GetPosition().y, light->GetPosition().z);
	glUniform1f(uniformFarPlane, light->GetFarPlane());
	omniShadowShader.SetLightMatrices(light->CalculateLightTransform());

	omniShadowShader.Validate();

	RenderScene();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPass(glm::mat4 projectionMatrix, glm::mat4 viewMatrix)
{

	glViewport(0, 0, 1366, 768);


	// Clear window
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	skybox.DrawSkybox(viewMatrix, projectionMatrix);

	shaderList[0].UseShader();

	uniformModel = shaderList[0].GetModelLocation();
	uniformProjection = shaderList[0].GetProjectionLocation();
	uniformView = shaderList[0].GetViewLocation();
	uniformEyePosition = shaderList[0].GetEyePositionLocation();
	uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
	uniformShininess = shaderList[0].GetShininessLocation();

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniform3f(uniformEyePosition, camera.GetCameraPosition().x, camera.GetCameraPosition().y, camera.GetCameraPosition().z);

	shaderList[0].SetDirectionalLight(&mainLight);
	shaderList[0].SetPointLights(pointLights, pointLightCount, 3, 0);
	shaderList[0].SetSpotLights(spotLights, spotLightCount, 3 + pointLightCount, pointLightCount);

	glm::mat4 mainLightTransform = mainLight.CalculateLightTransform();
	shaderList[0].SetDirectionalLightTransform(&mainLightTransform);

	mainLight.GetShadowMap()->Read(GL_TEXTURE2);
	shaderList[0].SetTexture(1);
	shaderList[0].SetDirectionalShadowMap(2);


	glm::vec3 lowerLight = camera.GetCameraPosition();
	lowerLight.y -= 0.5f;
	spotLights[0].SetFlash(lowerLight, camera.GetCameraDirection());

	shaderList[0].Validate();

	RenderScene();
}

int main()
{
	mainWindow = Window(1366, 768); //1280:1024, 1024:768
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), -90.f, 0.f, 5.f, 0.05f);

	brickTexture = Texture((char*)("textures/brick.png"));
	brickTexture.LoadTextureA();

	dirtTexture = Texture((char*)("textures/dirt.png"));
	dirtTexture.LoadTextureA();

	plainTexture = Texture((char*)("textures/plain.png"));
	plainTexture.LoadTextureA();

	shinyMaterial = Material(4.f, 256);
	matteMaterial = Material(0.3f, 4);

	xwing = Model();
	xwing.LoadModel("models/x-wing.obj");

	blackhawk = Model();
	blackhawk.LoadModel("models/uh60.obj");

	mainLight = DirectionalLight(
		2048, 2048,
		1.f,0.53f, 0.3f,
		0.1f, 0.9f,
		-10.f, -12.0f, 18.5f
	);

	pointLights[pointLightCount++] = PointLight(
		1024, 1024,
		0.01f, 100.f,
		0.f, 0.f, 1.f,
		0.0f, 1.0f,
		1.0f, 2.f, 0.f,
		0.3f, 0.2f, 0.1f
	);

	pointLights[pointLightCount++] = PointLight(
		1024, 1024,
		0.01f, 100.f,
		0.f, 1.f, 0.f,
		0.0f, 1.0f,
		-4.0f, 2.f, 0.f,
		0.3f, 0.2f, 0.1f
	);
	
	spotLights[spotLightCount++] = SpotLight(
		1024, 1024,
		0.01f, 100.f,
		1.f, 1.f, 1.f,
		0.0f, 2.f,
		0.f, 0.f, 0.f,
		0.0f, -1.f, 0.f,
		1.0f, 0.0f, 0.0f,
		20.f
	);

	spotLights[spotLightCount++] = SpotLight(
		1024, 1024,
		0.01f, 100.f,
		1.f, 1.f, 1.f,
		0.0f, 1.f,
		0.f, -1.5f, 0.f,
		-100.0f, -1.f, 0.f,
		1.0f, 0.0f, 0.0f,
		20.f
	);

	std::vector<std::string> skyboxFaces;

	skyboxFaces.push_back("textures/skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("textures/skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("textures/skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("textures/skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("textures/skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("textures/skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	glm::mat4 projection = glm::perspective(glm::radians(60.f), (GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(), 0.1f, 100.f);




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

		if (mainWindow.getKeys()[GLFW_KEY_F])
		{
			spotLights[0].Toggle();
			mainWindow.getKeys()[GLFW_KEY_F] = false;
		}

		DirectionalShadowMapPass(&mainLight);

		for (size_t i = 0; i< pointLightCount; i++)
		{
			OmniShadowMapPass(&pointLights[i]);
		}

		for (size_t i = 0; i < spotLightCount; i++)
		{
			OmniShadowMapPass(&spotLights[i]);
		}

		RenderPass(projection, camera.calculateViewMatrix());

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}