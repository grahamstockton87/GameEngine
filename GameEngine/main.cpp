#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT

#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <mat4x4.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/string_cast.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <memory>
#include <map>

#include "ft2build.h"
#include FT_FREETYPE_H

#include "CommonValues.h"

#include "Mesh.h"
#include "Window.h"
#include "Camera.h"
#include "Texture.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Material.h"
#include "Model.h"
#include "Skybox.h"
#include "Triangle.h"
#include "Sprite.h"
#include "AudioPlayer.h"
#include "CollisionUtils.h"
#include "Text.h"

const float toRadians = 3.14159265f / 180.0f;
const float toDegrees = 180.0f / 3.14159265f;
// lights class list
unsigned int pointLightCount = 0;
unsigned int spotLightCount = 0;

GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0, uniformSpecularIntensity = 0, uniformShininess = 0, uniformDirection = 0, uniformDiffuseIntensity = 0, uniformOmniLightPos = 0, uniformFarPlane = 0;

Window mainWindow;

Camera camera(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 5.0f, 20.0f, 5.0f, 45.0f);

std::vector<std::unique_ptr<Mesh>> meshList;

std::vector<Shader*> shaderList;
Shader directionalShadowShader;
Shader omniShadowShader;
Shader debugLine;
Shader debugBox;
Shader textShader;
Shader hudShader;

Texture brickTexture;
Texture transparent;
Texture wallpaper, tile, window;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

Material shinyMaterial;
Material dullMaterial;

Model dog;
Model land;

Skybox skybox;

Sprite healthBar;

TextRenderer fpsText;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

GLfloat Angle = 0.0f;

bool direction = true;
float triOffset = 0.0f;
float triMaxOffset = 0.6f;
float triIncrement = 0.005f;

bool  sizeDirection = true;
float curSize = 0.4f;
float maxSize = 0.8f;
float minSize = 0.1f;
float angle = 0;

bool ranOnce = false;

glm::vec3 dogPosition = glm::vec3(-5.0f, 0.0f, 0.0f);  // Initial dog world position
float dogHealth = 100.0f;
bool dogHit = false;


GLuint hudVAO, hudVBO;

std::map<GLchar, Character> Characters;

// Vertex Shader code
static const char* vShader = "Shaders/shaderVert.glsl";

// Fragment Shader
static const char* fShader = "Shaders/shaderFrag.glsl";

float health = 200;
const float maxHealth = 200;

void RenderScene() {
	// BOX
	// translate first then move order matters
	for (auto& mesh : meshList) {
		mesh->ModelReset();
	}

	//meshList[0]->translate(0.0f, 1.0f, 0.0f);
	//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(meshList[0]->GetModel()));
	//window.UseTexture();
	//shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	//meshList[0]->RenderMesh();
	////meshList[0]->updateVertices();
 //  // meshList[0]->box = meshList[0]->CalculateBoundingBox();


	//// PYRAMID
	//meshList[1]->translate(0.0f, 1.0f, -3.0f);
	//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(meshList[1]->GetModel()));
	//brickTexture.UseTexture();
	//dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	//meshList[1]->RenderMesh();
	////meshList[1]->updateVertices();
 //  // meshList[1]->box = meshList[1]->CalculateBoundingBox();


	//// FLOOR
	//meshList[2]->translate(0.0f, 0.1f, 0.0f);
	//meshList[2]->scale(10.0f, 0.5f, 10.0f);
	//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(meshList[2]->GetModel()));
	//tile.UseTexture();
	//shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	//meshList[2]->RenderMesh();
	////meshList[2]->updateVertices();
	////meshList[2]->box = meshList[2]->CalculateBoundingBox();

	//// reverse box
	//meshList[3]->translate(0.0f, 10.0f, 0.0f);
	//meshList[3]->scale(10.0f, 10.0f, 10.0f);
	//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(meshList[3]->GetModel()));
	//wallpaper.UseTexture();
	//shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	////meshList[3]->updateVertices();
	////meshList[3]->box = meshList[3]->CalculateBoundingBox();
	//meshList[3]->RenderMesh();

	//// ramp box
	//meshList[4]->translate(-2.0f, 4.5f, 9.0f);
	//meshList[4]->rotate(45, 0.0f, 0.0f, 1.0f);
	//meshList[4]->scale(10.0f, 2.0f, 1.0f);
	//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(meshList[4]->GetModel()));
	//tile.UseTexture();
	//shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	//meshList[4]->RenderMesh();
	//
	////meshList[4]->updateVertices();
	////meshList[4]->box = meshList[4]->CalculateBoundingBox();

	//// box2
	//meshList[5]->translate(7.0f, 12.5f, 0.0f);
	//meshList[5]->scale(3.5f, 0.5f, 10.0f);
	//
	//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(meshList[5]->GetModel()));
	//tile.UseTexture();
	//shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	//meshList[5]->RenderMesh();

	//meshList[5]->updateVertices();
	//meshList[5]->box = meshList[5]->CalculateBoundingBox();

	Angle += 0.1f * deltaTime;
	if (Angle > 360.0f) {
		Angle = 0.0f;
	}

	
	// DOG CHASING CAMERA
	float dogSpeed = 1.0f; // Units per second (tweak as needed)

	glm::vec3 target = camera.getCameraPostion();  // Camera/player position

	// Optional: keep dog grounded by matching camera Y or fixed Y
	//target.y = dogPosition.y; // Optional: only chase horizontally
	target.y -= camera.radiusY;
	glm::vec3 direction = target - dogPosition;
	float distance = glm::length(direction);

	// translate bounding box 
	if (distance > 0.1f) {  // Small threshold to stop jitter
		direction = glm::normalize(direction);
		dogPosition += direction * dogSpeed * deltaTime;
	}

	if (dog.IsValid) {
		dog.model = glm::mat4(1.0f);
		for (const auto& mesh : dog.GetMeshList())
			mesh->model = glm::mat4(1.0f);
		//meshList[6]->translate(dogPosition.x, dogPosition.y, dogPosition.z);
		dog.translate(dogPosition.x, dogPosition.y, dogPosition.z);
		dog.scale(0.1, 0.1, 0.1);
		//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(dog.GetMeshList()[0]->GetModel()));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(dog.model));
		//
		// std::cout << glm::to_string(dog.model) << std::endl;
		
		shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		dog.RenderModel(uniformModel);

		//dog.translate(dogPosition.x, dogPosition.y, dogPosition.z);
		glm::vec3 lookDir = glm::normalize(camera.getCameraPostion() - dogPosition);
		float angleY = atan2(lookDir.x, lookDir.z); // Yaw rotation to face player
		//dog.rotate(angleY * toDegrees, 0.0f, 1.0f, 0.0f);
		dog.rotate(angleY * toDegrees, 0.0, 1.0, 0.0);

		dog.CalculateModelSpaceBoundingBox();
	}

	// fix this 
	if (!ranOnce) {
		for (int i = 0; i < meshList.size(); i++) {
			meshList[i]->CalculateModelSpaceBoundingBox();
			meshList[i]->transformBoundingBox();
		}
		ranOnce = true;
	}

	// ROOM
	
	land.model = glm::mat4(1.0f);
	for (const auto& mesh : land.GetMeshList())
		mesh->model = glm::mat4(1.0f);
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(land.model));
	dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	tile.UseTexture();
	land.RenderModel(uniformModel, true);

	//std::cout << "Dog Box Min: " << glm::to_string(dog.GetBox().min)
	//	<< " Max: " << glm::to_string(dog.GetBox().max) << std::endl;
	glBindVertexArray(0);
}
void CreateShaders() {
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(shader1);

	directionalShadowShader = Shader();
	directionalShadowShader.CreateFromFiles("Shaders/directional_shadow_map_vert.glsl", "Shaders/directional_shadow_map_frag.glsl");

	omniShadowShader = Shader();
	omniShadowShader.CreateFromFiles("Shaders/omni_shadow_map_vert.glsl", "Shaders/omni_shadow_map_geom.glsl", "Shaders/omni_shadow_map_frag.glsl");

	debugLine = Shader();
	debugLine.CreateFromFiles("Shaders/debug_line_vert.glsl", "Shaders/debug_line_frag.glsl");

	debugBox = Shader();
	debugBox.CreateFromFiles("Shaders/debug_box_vert.glsl", "Shaders/debug_box_frag.glsl");

	textShader = Shader();
	textShader.CreateFromFiles("Shaders/font_shader_vert.glsl", "Shaders/font_shader_frag.glsl");

	hudShader = Shader();
	hudShader.CreateFromFiles("Shaders/hud_vert.glsl", "Shaders/hud_frag.glsl");
}
void calcAverageNormals(unsigned int* indices, unsigned int indicieCount, GLfloat* vertices,
	unsigned int verticeCount, unsigned int vLength, unsigned int normalOffset) {
	for (size_t i = 0; i < indicieCount; i += 3) {
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;

		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}
	for (size_t i = 0; i < verticeCount / vLength; i++) {
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}
void DirectionalShadowPass(DirectionalLight* light) {
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
void OmniShadowMapPass(PointLight* light) {
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
void RenderPass(glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {

	glViewport(0, 0, 1366, 768);
	// clear window
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	skybox.DrawSkybox(viewMatrix, projectionMatrix);

	shaderList[0]->UseShader();
	uniformModel = shaderList[0]->GetModelLocation();
	uniformProjection = shaderList[0]->GetProjectionLocation();
	uniformView = shaderList[0]->GetViewLocation();
	uniformEyePosition = shaderList[0]->GetEyePositionLocation();
	uniformSpecularIntensity = shaderList[0]->GetSpecularIntensityLocation();
	uniformShininess = shaderList[0]->GetShininessLocation();

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
	glUniform3f(uniformEyePosition, camera.getCameraPostion().x, camera.getCameraPostion().y, camera.getCameraPostion().z);

	shaderList[0]->SetDirectionalLight(&mainLight);
	shaderList[0]->SetPointLights(pointLights, pointLightCount, 3, 0);
	shaderList[0]->SetSpotLights(spotLights, spotLightCount, 3 + pointLightCount, pointLightCount);
	glm::mat4 lightTransform = mainLight.CalculateLightTransform();
	shaderList[0]->SetDirectionalLightTransform(&lightTransform);

	mainLight.GetShadowMap()->Read(GL_TEXTURE2);
	shaderList[0]->SetTexture(1);
	shaderList[0]->SetDirectionalShadowMap(2);

	glm::vec3 lowerLight = camera.getCameraPostion();
	lowerLight.y -= 0.1f;
	spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

	shaderList[0]->Validate();

	RenderScene();

}
void scaleUVs(GLfloat* vertices, int totalFloatCount, int scale) {
	int floatsPerVertex = 8;

	int numVertices = totalFloatCount / floatsPerVertex;

	for (int i = 0; i < numVertices; ++i) {
		int offset = i * floatsPerVertex;

		// UVs are at index 3 and 4
		vertices[offset + 3] *= scale; // U
		vertices[offset + 4] *= scale; // V
	}
}
void DrawBoundingBox(const BoundingBox& box, Shader& shader, const glm::mat4& projection, const glm::mat4& view)
{
	static GLuint VAO = 0, VBO = 0, EBO = 0;
	static bool initialized = false;

	if (!initialized) {
		// Only set up once
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 8, nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		unsigned int indices[] = {
			0, 1, 1, 2, 2, 3, 3, 0, // bottom
			4, 5, 5, 6, 6, 7, 7, 4, // top
			0, 4, 1, 5, 2, 6, 3, 7  // sides
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
		initialized = true;
	}

	// Calculate corners
	glm::vec3 corners[8] = {
		{box.min.x, box.min.y, box.min.z},
		{box.max.x, box.min.y, box.min.z},
		{box.max.x, box.max.y, box.min.z},
		{box.min.x, box.max.y, box.min.z},
		{box.min.x, box.min.y, box.max.z},
		{box.max.x, box.min.y, box.max.z},
		{box.max.x, box.max.y, box.max.z},
		{box.min.x, box.max.y, box.max.z}
	};

	// Upload corners
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(corners), corners);

	// Bind shader
	shader.UseShader();
	glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(shader.GetViewLocation(), 1, GL_FALSE, glm::value_ptr(view));

	// Draw
	glBindVertexArray(VAO);
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}


static double fpsLastTime = glfwGetTime();
static int frameCount = 0;
static double fps = 0.0;

double deltaLastTime = glfwGetTime();

int main() {

	mainWindow = Window(1200, 800);
	mainWindow.Initialize();

	// OBJECTS --------------------------------------------------------------------------------
	unsigned int pyramidIndices[]{
	0, 3, 1,
	1, 3, 2,
	2, 3, 0,
	0, 1, 2
	};
	GLfloat pyramidVertices[] = {
		//    x      y     z       u     v        N
			-1.0f, -1.0f, 0.0f,   0.0f, 0.0f,    0.0f,0.0f,0.0f,
			0.0f, -1.0f, 1.0f,    0.5f, 0.0f,    0.0f,0.0f,0.0f,
			1.0f, -1.0f, 0.0f,    1.0f, 0.0f,    0.0f,0.0f,0.0f,
			0.0f, 1.0f, 0.0f,     0.5f, 1.0f,    0.0f,0.0f,0.0f
	};
	calcAverageNormals(pyramidIndices, 12, pyramidVertices, 32, 8, 5);

	unsigned int boxIndices[] = {
		// front face (z = +1)
		0, 1, 2,  2, 3, 0,        // bottom-left, bottom-right, top-right, top-left

		// back face (z = -1)
		4, 5, 6,  6, 7, 4,        // needs reversed winding

		// left face (x = -1)
		8, 9, 10, 10, 11, 8,

		// right face (x = +1)
		12, 14, 13, 14, 12, 15,   // fixed: CCW winding

		// top face (y = +1)
		16, 18, 17, 18, 16, 19,   // fixed: CCW winding

		// bottom face (y = -1)
		20, 21, 22, 22, 23, 20
	};
	GLfloat boxVertices[] = {
		// FRONT (+Z)
		-1, -1,  1,   0, 0,   0,  0, -1,
		 1, -1,  1,   5, 0,   0,  0, -1,
		 1,  1,  1,   5, 5,   0,  0, -1,
		-1,  1,  1,   0, 5,   0,  0, -1,

		// BACK (-Z)
		 1, -1, -1,   0, 0,   0,  0, 1,
		-1, -1, -1,   5, 0,   0,  0, 1,
		-1,  1, -1,   5, 5,   0,  0, 1,
		 1,  1, -1,   0, 5,   0,  0, 1,

		 // LEFT (-X)
		 -1, -1, -1,   0, 0,   1,  0,  0,
		 -1, -1,  1,   5, 0,   1,  0,  0,
		 -1,  1,  1,   5, 5,   1,  0,  0,
		 -1,  1, -1,   0, 5,   1,  0,  0,

		 // RIGHT (+X)
		  1, -1,  1,   0, 0,  -1,  0,  0,
		  1, -1, -1,   5, 0,  -1,  0,  0,
		  1,  1, -1,   5, 5,  -1,  0,  0,
		  1,  1,  1,   0, 5,  -1,  0,  0,

		  // TOP (+Y)
		  -1,  1,  1,   0, 0,   0, -1,  0,
		   1,  1,  1,   5, 0,   0, -1,  0,
		   1,  1, -1,   5, 5,   0, -1,  0,
		  -1,  1, -1,   0, 5,   0, -1,  0,

		  // BOTTOM (-Y)
		  -1, -1, -1,   0, 0,   0, 1,  0,
		   1, -1, -1,   5, 0,   0, 1,  0,
		   1, -1,  1,   5, 5,   0, 1,  0,
		  -1, -1,  1,   0, 5,   0, 1,  0
	};

	GLfloat boxVertices2[] = {
		//   X     Y     Z      U   V     NX   NY   NZ
		-1, -1, -1,   0, 0,    0,  0,  0,  // 0: Left  Bottom Back
		 1, -1, -1,   1, 0,    0,  0,  0,  // 1: Right Bottom Back
		 1,  1, -1,   1, 1,    0,  0,  0,  // 2: Right Top    Back
		-1,  1, -1,   0, 1,    0,  0,  0,  // 3: Left  Top    Back
		-1, -1,  1,   0, 0,    0,  0,  0,  // 4: Left  Bottom Front
		 1, -1,  1,   1, 0,    0,  0,  0,  // 5: Right Bottom Front
		 1,  1,  1,   1, 1,    0,  0,  0,  // 6: Right Top    Front
		-1,  1,  1,   0, 1,    0,  0,  0   // 7: Left  Top    Front
	};
	unsigned int boxIndices2[] = {
		// Back face (CCW)
		2, 1, 0,
		0, 3, 2,

		// Front face
		4, 5, 6,
		6, 7, 4,

		// Left face
		7, 4, 0,
		0, 3, 7,

		// Right face
		1, 5, 6,
		6, 2, 1,

		// Top face
		6, 7, 3,
		3, 2, 6,

		// Bottom face
		0, 4, 5,
		5, 1, 0
	};

	GLfloat verticesReverseBox[] = {
		// FRONT (+Z) → normal: (0, 0, 1)
		-1, -1,  1,   0, 0,   0,  0, 1,
		 1, -1,  1,   10, 0,   0,  0, 1,
		 1,  1,  1,   10, 10,   0,  0, 1,
		-1,  1,  1,   0, 10,   0,  0, 1,

		// BACK (-Z) → normal: (0, 0, -1)
		 1, -1, -1,   0, 0,   0,  0, -1,
		-1, -1, -1,   10, 0,   0,  0, -1,
		-1,  1, -1,   10, 10,   0,  0, -1,
		 1,  1, -1,   0, 10,   0,  0, -1,

		 // LEFT (-X) → normal: (-1, 0, 0)
		 -1, -1, -1,   0, 0,  -1,  0,  0,
		 -1, -1,  1,   10, 0,  -1,  0,  0,
		 -1,  1,  1,   10, 10,  -1,  0,  0,
		 -1,  1, -1,   0, 10,  -1,  0,  0,

		 // RIGHT (+X) → normal: (1, 0, 0)
		  1, -1,  1,   0, 0,   1,  0,  0,
		  1, -1, -1,   10, 0,   1,  0,  0,
		  1,  1, -1,   10, 10,   1,  0,  0,
		  1,  1,  1,   0, 10,   1,  0,  0,

		  // TOP (+Y) → normal: (0, 1, 0)
		  -1,  1,  1,   0, 0,   0,  1,  0,
		   1,  1,  1,   10, 0,   0,  1,  0,
		   1,  1, -1,   10, 10,   0,  1,  0,
		  -1,  1, -1,   0, 10,   0,  1,  0,

		  // BOTTOM (-Y) → normal: (0, -1, 0)
		  -1, -1, -1,   0, 0,   0, -1,  0,
		   1, -1, -1,   10, 0,   0, -1,  0,
		   1, -1,  1,   10, 10,   0, -1,  0,
		  -1, -1,  1,   0, 10,   0, -1,  0
	};
	unsigned int indicesBoxReverse[] = {
		// front face (+Z)
		2, 1, 0,   0, 3, 2,

		// back face (-Z)
		6, 5, 4,   4, 7, 6,

		// left face (-X)
		10, 9, 8,  8, 11, 10,

		// right face (+X)
		14, 13, 12,  12, 15, 14,

		// top face (+Y)
		18, 17, 16,  16, 19, 18,

		// bottom face (-Y)
		21, 22, 20,  20, 22, 23
	};
	//calcAverageNormals(indicesBoxReverse, 36, verticesReverseBox, 192, 8, 5);

	unsigned int floorIndices[]{
		0, 2, 1,
		1, 2, 3
	};
	GLfloat floorVertices[] = {
		-10.0f, 0.05f, -10.0f,   0.0f, 0.0f,     0.0f, -1.0f, 0.0f,
		10.0f, 0.05f, -10.0f,    10.0f, 0.0f,    0.0f, -1.0f, 0.0f,
		-10.0f, 0.05f, 10.0f,    0.0f, 10.0f,    0.0f, -1.0f, 0.0f,
		10.0f, 0.05f, 10.0f,     10.0f, 10.0f,   0.0f, -1.0f, 0.0f
	};

	unsigned int indicesSprite[] = { 0, 1, 2, 2, 3, 0 };
	GLfloat verticesSprite[] = {
		// positions      // texCoords
		mainWindow.getBufferWidth() / 2 - 200, 50.0f,     0.0f, 0.0f,
		mainWindow.getBufferWidth() / 2, 50.0f,    1.0f, 0.0f,
		mainWindow.getBufferWidth() / 2, 150.0f  ,    1.0f, 1.0f,
		mainWindow.getBufferWidth() / 2 - 200, 150.0f,     0.0f, 1.0f
	};


	healthBar = Sprite(verticesSprite, indicesSprite, 16, 6);
	healthBar.CreateSprite();


	//std::unique_ptr<Mesh> cube = std::make_unique<Mesh>(boxVertices, boxIndices, 192, 36);
	//cube->CreateMesh();
	//meshList.push_back(std::move(cube));

	//std::unique_ptr<Mesh> pyramid = std::make_unique<Mesh>(pyramidVertices, pyramidIndices, 32, 12);
	//pyramid->CreateMesh();
	//meshList.push_back(std::move(pyramid));

	//int totalFloats = sizeof(boxVertices) / sizeof(GLfloat);
	//scaleUVs(boxVertices, totalFloats, 5);
	//std::unique_ptr<Mesh> floor = std::make_unique<Mesh>(boxVertices, boxIndices, 192, 36);
	//floor->CreateMesh();
	//meshList.push_back(std::move(floor));

	//std::unique_ptr<Mesh> cubeReverse = std::make_unique<Mesh>(verticesReverseBox, indicesBoxReverse, 192, 36);
	//cubeReverse->CreateMesh();
	//cubeReverse->moveToTopOfBox = false;
	//meshList.push_back(std::move(cubeReverse));

	//std::unique_ptr<Mesh> ramp = std::make_unique<Mesh>(boxVertices, boxIndices, 192, 36);
	//ramp->CreateMesh();
	//ramp->ID = "ramp";
	//ramp->UsesBoxCollision = false;
	//meshList.push_back(std::move(ramp));

	//std::unique_ptr<Mesh> cube3 = std::make_unique<Mesh>(boxVertices, boxIndices, 192, 36);
	//cube3->CreateMesh();
	//meshList.push_back(std::move(cube3));

	CreateShaders();

	// RAYS
	GLuint rayVAO = 0, rayVBO = 0;

	glGenVertexArrays(1, &rayVAO);
	glGenBuffers(1, &rayVBO);

	glBindVertexArray(rayVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rayVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

// LOAD FONTS ----------------------------------------------------------------------------------------------------------
	if (!fpsText.LoadFont("fonts/arial.ttf")) {
		std::cerr << "Could not load font.\n";
		return -1;
	}

// MATERIALS  -------------------------------------------------------------------------------------------
	shinyMaterial = Material(1.0f, 128);
	dullMaterial = Material(0.3f, 4);

// LOAD TEXTURES --------------------------------------------------------------------------------------------
	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();

	transparent = Texture("Textures/transparent.png");
	transparent.LoadTextureA();

	tile = Texture("Textures/tile.png");
	tile.LoadTexture();

	wallpaper = Texture("Textures/wallpaper.jpg");
	wallpaper.LoadTexture();

	window = Texture("Textures/window2.png");
	window.LoadTextureA();

	dog = Model();
	dog.LoadModel("Models/dog.obj");

	land = Model();
	land.LoadModel("Models/gamelevel.obj");

	//for (const auto& mesh : dog.GetMeshList()) {
	//	meshList.push_back(std::make_unique<Mesh>(*mesh));
	//}

	// LIGHTS --------------------------------------------------------------------------------------------
	mainLight = DirectionalLight(1024 * 2, 1024 * 2,
		1.0f, 1.0f, 1.0f,
		0.1f, 0.1f,
		0.0f, -7.0f, -1.0f);

	pointLights[0] = PointLight(1024, 1024,
		0.1f, 100.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f,
		-4.0f, 2.0f, 0.0f,
		0.3f, 0.1f, 0.1f);
	pointLightCount++;
	pointLights[1] = PointLight(1024, 1024,
		0.1f, 100.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f,
		4.0f, 2.0f, 0.0f,
		0.3f, 0.1f, 0.1f);
	pointLightCount++;


	spotLights[0] = SpotLight(1024, 1024,
		0.1f, 100.0f,
		1.0f, 1.0f, 1.0f, // color
		0.0f, 5.0f, // ambient diffuse
		4.0f, 1.0f, 0.0f, // position
		0.0f, -1.0f, 0.0f, // direction
		0.3f, 0.1f, 0.1f, // Equation
		10.0f); // Angle
	spotLightCount++;

	std::vector<std::string> skyBoxFaces;
	skyBoxFaces.push_back("Skyboxes/Skybox/cupertin-lake_rt.tga");
	skyBoxFaces.push_back("Skyboxes/Skybox/cupertin-lake_lf.tga");
	skyBoxFaces.push_back("Skyboxes/Skybox/cupertin-lake_up.tga");
	skyBoxFaces.push_back("Skyboxes/Skybox/cupertin-lake_dn.tga");
	skyBoxFaces.push_back("Skyboxes/Skybox/cupertin-lake_bk.tga");
	skyBoxFaces.push_back("Skyboxes/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyBoxFaces);

	Assimp::Importer importer;
	// loop until know closed

	AudioPlayer player;

	if (!player.LoadMP3("audio/test.mp3")) {
		return -1;
	}

	player.Play();
	std::cout << "Playing MP3..." << std::endl;

	while (!mainWindow.getShouldClose()) {

		float maxGroundLevel = 0.0f;
		bool anyGrounded = false;

		float closestY = -FLT_MAX;
		bool hit = false;
		bool hitSide = false;
		bool hitTop = false;
		dogHit = false;
		// clear window
		camera.previousPosition = camera.position;

		double currentTime = glfwGetTime();
		frameCount++;

		if (currentTime - fpsLastTime >= 1.0) {
			fps = frameCount / (currentTime - fpsLastTime);
			frameCount = 0;
			fpsLastTime = currentTime;
		}
		double now = glfwGetTime();
		deltaTime = now - deltaLastTime;
		deltaLastTime = now;


		// get handle user event inputs
		glfwPollEvents();

		if (dogHealth <= 0) {
			dog.IsValid = false;
			dog.ClearModel();
		}
			

		//update();
		glm::mat4 projection = glm::perspective(glm::radians(camera.getFov()), (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);
		

		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange(), mainWindow.getYScrollChange(), mainWindow.getLeftClicked(), deltaTime);

// SHOOT
		if (mainWindow.getLeftClicked()) {

			glm::vec3 rayOrigin = glm::vec3(camera.getCameraPostion());
			glm::vec3 rayDirection = camera.getCameraDirection(); // aleady normalized

			float maxDistance = 4.0f;
			// Optional: draw debug line
			glm::vec3 rayStart = rayOrigin;
			glm::vec3 rayEnd = rayOrigin + rayDirection * maxDistance;
			glm::vec3 rayVerts[2] = { rayStart, rayEnd };

			glBindBuffer(GL_ARRAY_BUFFER, rayVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(rayVerts), rayVerts);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Perform AABB intersection checks
			for (auto& obj : meshList) {
				if (obj->shootable) {
					auto box = obj->getBoundingBox();
					if (RayIntersectsAABB(rayOrigin, rayDirection, box, maxDistance)) {
						obj->ClearMesh();
						break;
					}
				}
			}
			// Ray hit test...
			if (RayIntersectsAABB(rayOrigin, rayDirection, dog.GetBox(), maxDistance)) {
				dogHealth -= 100.0f;
			}
		}

		glm::vec3 delta = camera.position - camera.previousPosition;

		const float groundSnapOffset = 0.001f; // small buffer to avoid falling through
		float groundY = 0.0f;
// MESH LIST OBJECT INTERSECTION CHECK
		for (int i = 0; i < meshList.size(); i++) {
			if (meshList[i]->IsValid) {
				if (!meshList[i]->UsesBoxCollision) {
					// if the player doesnt intersect the triangle then use box collsion
					CheckTriangleCollsion(closestY, hit, hitSide, hitTop, meshList[i], camera);
				}
				else {
					CheckBoxCollision(hit, hitSide, closestY, groundY, delta, meshList[i], camera, deltaTime);
				}
			}
		}

		if (dog.IsValid) {
			CheckBoxCollision(dogHit, hitSide, closestY, groundY, delta, dog.GetBox(), true, camera, deltaTime);
			if (dogHit && health > 0) {
				health -= 10.0f * deltaTime;
			}
		}

		for (auto& mesh : land.GetMeshList())
			CheckTriangleCollsion(closestY, hit, hitSide, hitTop, mesh, camera);

		GroundPlayer(hit, anyGrounded, closestY, camera);

		camera.keyControl(mainWindow.getKeys(), deltaTime);

		camera.isGrounded = anyGrounded;

		if (anyGrounded) {
			camera.groundLevel = maxGroundLevel;
			camera.position.y = camera.groundLevel + camera.radiusY;
			camera.verticalVelocity = 0.0f;
			camera.isGrounded = true;
		}
		else {
			camera.groundLevel = 0.0f;  // Reset to floor height
		}
		if (hitSide) {
			glm::vec3 moveDelta = camera.position - camera.previousPosition;

			// Resolve only the axis that caused the larger movement
			if (std::abs(moveDelta.x) > std::abs(moveDelta.z)) {
				// Restrict X movement only
				camera.position.x = camera.previousPosition.x;
			}
			else {
				// Restrict Z movement only
				camera.position.z = camera.previousPosition.z;
			}
		}

		//std::cout << health << std::endl;
		healthBar.mVertices[0] = mainWindow.getBufferWidth() / 2 - health;
		healthBar.mVertices[12] = mainWindow.getBufferWidth() / 2 - health;
		healthBar.CreateSprite();

		camera.updatePhysics(deltaTime);

		DirectionalShadowPass(&mainLight);
		// put into one list polymorpism
		for (size_t i = 0; i < pointLightCount; i++) {
			OmniShadowMapPass(&pointLights[i]);
		}
		for (size_t i = 0; i < spotLightCount; i++) {
			OmniShadowMapPass(&spotLights[i]);
		}
		RenderPass(projection, camera.calculateViewMatrix());

		// TEXT RENDER
		glm::mat4 orthoProjection = glm::ortho(0.0f, static_cast<float>(mainWindow.getBufferWidth()),
			0.0f, static_cast<float>(mainWindow.getBufferHeight()));

		std::string fpsTextValue = "FPS: " + std::to_string(static_cast<int>(fps));
		//std::cout << fps << std::endl;
		fpsText.RenderText(textShader, fpsTextValue, 10.0f, mainWindow.getBufferHeight() - 30.0f, 0.4f, glm::vec3(1.0f, 1.0f, 1.0f), orthoProjection);

		// DEBUG LINE RAY
		debugLine.UseShader();

		glUniformMatrix4fv(debugLine.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(debugLine.GetViewLocation(), 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

		glBindVertexArray(rayVAO);
		glDrawArrays(GL_LINES, 0, 2);
		glBindVertexArray(0);

		glUseProgram(0);

		// Bounding box debug 
		debugBox.UseShader();
		for (const auto& mesh : meshList)
			DrawBoundingBox(mesh->box, debugBox, projection, camera.calculateViewMatrix());
		DrawBoundingBox(dog.GetBox(), debugBox, projection, camera.calculateViewMatrix());
		BoundingBox b = dog.GetBox();
		//std::cout << "Box Min: " << glm::to_string(b.min) << ", Max: " << glm::to_string(b.max) << std::endl;

		
		// Prepare HUD pass
		hudShader.UseShader();
		glUniformMatrix4fv(hudShader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(orthoProjection));
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		glActiveTexture(GL_TEXTURE0);
		brickTexture.UseTexture(GL_TEXTURE0);
		glUniform1i(hudShader.GetUniformSpriteTextureLocation(), 0);
		healthBar.RenderSprite();

		glEnable(GL_DEPTH_TEST);


		mainWindow.swapBuffers();

		//glDisable(GL_BLEND);
		glUseProgram(0);  // <-- Unbind HUD shader
	}
	return 0;
}