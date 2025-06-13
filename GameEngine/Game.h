// Game.h
#ifndef GAME_H
#define GAME_H

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
#include <algorithm>

#include "ft2build.h"
#include FT_FREETYPE_H

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
#include "RayRenderer.h"
#include "TextRenderer.h"
#include "HUD.h"
#include "RenderUtils.h"

class Game {
public:
	Game();
	~Game();
	bool Initialize();
	void Run();
	void RenderScene();
private:
	void Update();
	void ProcessInput();
	void Update(float deltaTime);
	void Shoot();
	void CalcAverageNormals(unsigned int* indices, unsigned int indicieCount, GLfloat* vertices, unsigned int verticeCount, unsigned int vLength, unsigned int normalOffset);
	void CheckCollision();
	void GroundPlayer();
	void CreateShaders();
	void DirectionalShadowPass(DirectionalLight* light);
	void OmniShadowMapPass(PointLight* light);
	void RenderPass(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
	void scaleUVs(GLfloat* vertices, int totalFloatCount, int scale);
	void DrawBoundingBox(const BoundingBox& box, Shader& shader, const glm::mat4& projection, const glm::mat4& view);
	void initDOF();
	void updateFirstFrame();
	//void InitDepthOfFieldFBO();

	// Variables
	Window mainWindow;
	float centerX = mainWindow.getBufferWidth() / 2.0f; // Center of the screen
	float rightX = centerX;
	float currentWidth = 0;
	int screenW = mainWindow.getBufferWidth();
	int screenH = mainWindow.getBufferHeight();
	// TEXT RENDER
  

	Camera camera = Camera(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 5.0f, 20.0f, 5.0f, 45.0f);
	glm::vec3 delta = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 projection;

	std::vector<std::unique_ptr<Mesh>> meshList;
	std::vector<std::unique_ptr<Model>> modelList;
	std::vector<Shader*> shaderList;

	const float toRadians = 3.14159265f / 180.0f;
	const float toDegrees = 180.0f / 3.14159265f;
	// lights class list
	unsigned int pointLightCount = 0;
	unsigned int spotLightCount = 0;

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0, uniformSpecularIntensity = 0, uniformShininess = 0, uniformDirection = 0, uniformDiffuseIntensity = 0, uniformOmniLightPos = 0, uniformFarPlane = 0;

	Shader directionalShadowShader;
	Shader omniShadowShader;
	Shader motionBlurShader;
	Shader finalBlitShader;
	int currentFBO = 0;
	int nextFBO = 1;

	
	Shader RayShader;
	Shader debugBox;
	Shader textShader;
	Shader hudShader;
	Shader depthOfFieldShader;
	// — your post?process FBOs & textures —
	GLuint sceneFBO = 0;
	GLuint sceneColorTex = 0;
	GLuint sceneDepthTex = 0;

	// DOF target FBO
	GLuint dofFBO = 0;
	GLuint dofColorTex = 0;
	GLuint dofDepthTex = 0;

	// Blending & DOF parameters
	const float blurFactor = 4.0f;
	const float focalDistance = 10.0f;
	const float focalRange = 2.0f;
	const float maxBlur = 100.0f;

	Texture brickTexture;
	Texture transparent;
	Texture wallpaper, tile, window, healthBarTexture;

	DirectionalLight mainLight;
	PointLight pointLights[MAX_POINT_LIGHTS];
	SpotLight spotLights[MAX_SPOT_LIGHTS];

	Material shinyMaterial;
	Material dullMaterial;

	std::unique_ptr<Model> dog;
	std::unique_ptr<Model> land;

	Skybox skybox;

	Sprite healthBar;

	TextRenderer fpsText;
	RayRenderer rayRenderer; 
	HUD healthHUD;

	GLfloat deltaTime = 0.0f;
	GLfloat lastTime = 0.0f;

	GLfloat Angle = 0.0f;

	bool ranOnce = false;

	glm::vec3 dogPosition = glm::vec3(-20.0f, 10.0f, 10.0f);  // Initial dog world position
	float dogHealth = 100.0f;
	bool dogHit = false;

	GLuint hudVAO, hudVBO;


	// Vertex Shader code
	static constexpr const char* vShader = "Shaders/shaderVert.glsl";
	// Fragment Shader
	static constexpr const char* fShader = "Shaders/shaderFrag.glsl";

	float health = 200;
	const float maxHealth = 200;

	double fpsLastTime = glfwGetTime();
	int frameCount = 0;
	double fps = 0.0;

	bool firstFrame = true;

	double deltaLastTime = glfwGetTime();

	float spriteWidth = 400.0f;
	float spriteHeight = 20.0f;

	float windowWidth = static_cast<float>(mainWindow.getBufferWidth());
	float windowHeight = static_cast<float>(mainWindow.getBufferHeight());

	float xCenter = windowWidth / 2.0f;
	float yTop = windowHeight - 10.0f;

	float xDisplacement = 200.0f;

	// Collision detection variables
	float maxGroundLevel = 0.0f;
	bool anyGrounded = false;

	float closestY = -FLT_MAX;
	bool hit = false;
	bool hitSide = false;
	bool hitTop = false;

	const float groundSnapOffset = 0.001f; // small buffer to avoid falling through
	float groundY = 0.0f;

	glm::vec3 wallNormal = glm::vec3(0.0f);

	int debugDisplayMode = 0; // 0 = blended, 1 = previous, 2 = current

};
#endif // !GAME_H