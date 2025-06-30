#include "Game.h"



#include <cstddef>
#include <GL/glew.h>

Game::Game()
{
}
Game::~Game()
{
}
void Game::initDOF() {
	screenW = mainWindow.getBufferWidth();
	screenH = mainWindow.getBufferHeight();

	// ------------------ DOF FBO ------------------
	glGenFramebuffers(1, &dofFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, dofFBO);

	// Color texture for DOF output
	glGenTextures(1, &dofColorTex);
	glBindTexture(GL_TEXTURE_2D, dofColorTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screenW, screenH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dofColorTex, 0);

	// Optional depth (not needed unless you're rendering geometry here — which you're not)
	glGenTextures(1, &dofDepthTex);
	glBindTexture(GL_TEXTURE_2D, dofDepthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screenW, screenH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dofDepthTex, 0);

	// Only drawing to the color buffer
	GLenum drawBufs1[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBufs1);

	// Check completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR: DOF FBO is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ------------------ SCENE FBO ------------------
	glGenFramebuffers(1, &sceneFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

	// Scene color texture
	glGenTextures(1, &sceneColorTex);
	glBindTexture(GL_TEXTURE_2D, sceneColorTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screenW, screenH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneColorTex, 0);

	// Scene depth texture
	glGenTextures(1, &sceneDepthTex);
	glBindTexture(GL_TEXTURE_2D, sceneDepthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screenW, screenH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sceneDepthTex, 0);

	GLenum drawBufs2[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBufs2);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR: Scene FBO is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Final error check
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
		std::cerr << "OpenGL Error in initDOF: " << err << std::endl;
}
bool Game::Initialize() {
// Setup Window


	mainWindow = Window(1200, 800);
	mainWindow.Initialize();

// Fonts
	if (!fpsText.LoadFont("fonts/arial.ttf")) {
		std::cerr << "Could not load font.\n";
		return -1;
	}

// Load MATERIALS  -------------------------------------------------------------------------------------------
	shinyMaterial = Material(1.0f, 128);
	dullMaterial = Material(0.3f, 4);

// LOAD TEXTURES --------------------------------------------------------------------------------------------
	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();

	tile = Texture("Textures/tile.png");
	tile.LoadTexture();

	wallpaper = Texture("Textures/wallpaper.jpg");
	wallpaper.LoadTexture();

	window = Texture("Textures/window2.png");
	window.LoadTextureA();

	healthBarTexture = Texture("Textures/healthBar.jpg");
	healthBarTexture.LoadTexture();

	handAnimationTexture = Texture("Textures/gunAnimation.png");
	handAnimationTexture.LoadTextureA();

	crosshairTexture = Texture("Textures/crosshair.png");
	crosshairTexture.LoadTextureA();

	dog = std::make_unique<Model>();
	dog->LoadModel("Models/dog.obj");
	dog->rigid = false;
	dog->UsesBoxCollision = true;
	modelList.push_back(std::move(dog));

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			land = std::make_unique<Model>();
			land->LoadModel("Models/LevelDesignBase2.obj");
			land->scaleUVs(20.0f);
			land->model = glm::mat4(1.0f);
			land->scale(3.0f, 3.0f, 3.0f);
			land->translate(0.0f, 0.0f, i * 10.0f);
			land->SetRigid(true);
			land->UsesBoxCollision = false; // Disable box collision for this model
			modelList.push_back(std::move(land));
		}
	}

	
	for (auto& mesh : modelList[1]->GetMeshList()) {
		mesh->moveToTopOfBox = true;
	}


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
	CalcAverageNormals(pyramidIndices, 12, pyramidVertices, 32, 8, 5);

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

	GLfloat SpriteVertices[] = {
		//   x                                       ,   y                ,  u   ,  v
		// bottom‐left (on screen) → top of texture
		mainWindow.getBufferWidth() - spriteWidth, 0.0f                   , 0.0f, 1.0f,
		// bottom‐right         → top of texture
		mainWindow.getBufferWidth(),              0.0f                   , 1.0f, 1.0f,
		// top‐right            → bottom of texture
		mainWindow.getBufferWidth(),              spriteHeight           , 1.0f, 0.0f,
		// top‐left             → bottom of texture
		mainWindow.getBufferWidth() - spriteWidth, spriteHeight           , 0.0f, 0.0f,
	};

	const GLuint SpriteIndices[] = {
		0, 1, 2,
		2, 3, 0
	};
	// Define your sprite size
	float spriteWidth = 50.0f;
	float spriteHeight = 50.0f;

	float centerX = mainWindow.getBufferWidth() / 2.0f;
	float centerY = mainWindow.getBufferHeight() / 2.0f;

	GLfloat SpriteVerticesCentered[] = {
		// x                         y                          u     v
		// Bottom-left
		centerX - spriteWidth / 2,  centerY - spriteHeight / 2, 0.0f, 0.0f,
		// Bottom-right
		centerX + spriteWidth / 2,  centerY - spriteHeight / 2, 1.0f, 0.0f,
		// Top-right
		centerX + spriteWidth / 2,  centerY + spriteHeight / 2, 1.0f, 1.0f,
		// Top-left
		centerX - spriteWidth / 2,  centerY + spriteHeight / 2, 0.0f, 1.0f,
	};


	healthBar = Sprite(verticesSprite, indicesSprite, 16, 6);
	healthBar.CreateSprite();
	healthBar.SetTexture(&healthBarTexture);

	handAnimationSprite = Sprite(SpriteVertices, indicesSprite, 16, 6);
	//handAnimationSprite.SetFrameUVs(1, 5);
	handAnimationSprite.SetTexture(&handAnimationTexture);
	handAnimationSprite.CreateSprite();

	handAnimation = AnimationSprite(handAnimationSprite, 2, 0.01f);

	crosshairSprite = Sprite(SpriteVerticesCentered, indicesSprite, 16, 6);
	crosshairSprite.SetTexture(&crosshairTexture);
	crosshairSprite.CreateSprite();

	std::unique_ptr<Mesh> ramp = std::make_unique<Mesh>(boxVertices, boxIndices, 192, 36);
	ramp->CreateMesh();
	ramp->ID = "ramp";
	ramp->UsesBoxCollision = false;
	//meshList.push_back(std::move(ramp));

	CreateShaders(); // Make sure this creates RayShader first!
	rayRenderer.Init(); // Now safe to initialize

	// LIGHTS --------------------------------------------------------------------------------------------
	mainLight = DirectionalLight(1024 * 2, 1024 * 2,
		1.0f, 1.0f, 1.0f,
		0.1f, 1.0f,
		0.0f, -7.0f, -1.0f);
	spotLights[0] = SpotLight(1024, 1024,
		0.1f, 100.0f,
		1.0f, 1.0f, 1.0f, // color
		0.0f, 5.0f, // ambient diffuse
		4.0f, 1.0f, 0.0f, // position
		0.0f, -1.0f, 0.0f, // direction
		0.3f, 0.1f, 0.1f, // Equation
		10.0f); // Angle
	spotLightCount++;

	spotLights[1] = SpotLight(1024, 1024,
		0.1f, 100.0f,
		1.0f, 1.0f, 1.0f, // color
		0.0f, 50.0f, // ambient diffuse
		0.0f, 1.0f, 0.0f, // position
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

	//skybox = Skybox(skyBoxFaces);
	forestSkyboxTexture = Texture("Skyboxes/forest.png");
	forestSkyboxTexture.LoadTexture();
	forestSkybox = Skybox(forestSkyboxTexture.GetTextureID());
	std::cout << forestSkyboxTexture.GetTextureID();

	Assimp::Importer importer;
	// loop until know closed
	//AudioPlayer player;

	//if (!player.LoadMP3("audio/test.mp3")) {
	///	return -1;
	//}

	if (!gunAudioPlayer.LoadMP3("audio/gun.mp3")) {
		std::cerr << "Failed to load gun audio." << std::endl;
		return -1;
	}

	//player.Play();
	std::cout << "Playing MP3..." << std::endl;

	mainWindow.InitMotionBlurFBO(mainWindow.getBufferWidth(), mainWindow.getBufferHeight());
	screenW = mainWindow.getBufferWidth();
	screenH = mainWindow.getBufferHeight();
	initDOF();

}
void Game::Run() {
	// Retrieve Motion Blur FBOs
	const GLuint mbSceneFBO = mainWindow.GetMotionBlurFBO(0);
	const GLuint mbHistoryFBO = mainWindow.GetMotionBlurFBO(1);

	bool firstFrame = true;

	// One-time clear so history starts empty
	for (GLuint fbo : { mbSceneFBO, mbHistoryFBO }) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	while (!mainWindow.getShouldClose()) {
		// ----- 1) Update game state -----
		int screenW = mainWindow.getBufferWidth();
		int screenH = mainWindow.getBufferHeight();
		closestY = -FLT_MAX;
		dogHit = false;
		wallNormal = glm::vec3(0.0f);
		delta = camera.position - camera.previousPosition;
		Update();
		CheckCollision();
		ProcessInput();
		GroundPlayer();
		Shoot();
		camera.updatePhysics(deltaTime);

		// ----- 2) Prepare projections -----
		orthoProj = glm::ortho(0.f, float(screenW), 0.f, float(screenH));
		perspProj = glm::perspective(
			glm::radians(camera.getFov()),
			float(screenW) / float(screenH),
			0.1f, 100.f
		);

		// ----- 3) Render scene to sceneFBO -----
		glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
		glViewport(0, 0, screenW, screenH);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderPass(perspProj, camera.calculateViewMatrix());

		// ----- 4) Depth-of-Field pass: scene → dofFBO -----
		glBindFramebuffer(GL_FRAMEBUFFER, dofFBO);
		glViewport(0, 0, screenW, screenH);
		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT);

		depthOfFieldShader.UseShader();
		depthOfFieldShader.SetFocalDistance(focalDistance);
		depthOfFieldShader.SetFocalRange(focalRange);
		depthOfFieldShader.SetMaxBlur(maxBlur);
		glm::vec2 texelSize(1.f / screenW, 1.f / screenH);
		glUniform2f(
			depthOfFieldShader.GetUniformLocation("texelSize"),
			texelSize.x, texelSize.y
		);
		// bind inputs
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sceneColorTex);
		glUniform1i(
			depthOfFieldShader.GetUniformLocation("sceneColor"), 0
		);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, sceneDepthTex);
		glUniform1i(
			depthOfFieldShader.GetUniformLocation("sceneDepth"), 1
		);
		// execute DOF
		RenderUtils::DrawFullScreenQuad(
			depthOfFieldShader,
			sceneColorTex, "sceneColor",
			sceneDepthTex, "sceneDepth"
		);

		// ----- 5) Motion Blur pass: dofColorTex + history → mbHistoryFBO -----
		glBindFramebuffer(GL_FRAMEBUFFER, mbHistoryFBO);
		glViewport(0, 0, screenW, screenH);
		glDisable(GL_DEPTH_TEST);

		motionBlurShader.UseShader();
		motionBlurShader.SetMixFactor(0.7f); // Adjust mix factor as needed
		// blend: unit0 = DOF result, unit1 = previous history (or DOF on first frame)
		RenderUtils::DrawFullScreenQuad(
			motionBlurShader,
			dofColorTex, "currentFrame",
			firstFrame ? dofColorTex : mainWindow.GetMotionBlurTexture(1),
			"previousFrame"
		);

		// ----- 6) Final blit: mbHistory → screen -----
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenW, screenH);
		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT);

		finalBlitShader.UseShader();
		// bind accumulated texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mainWindow.GetMotionBlurTexture(1));
		RenderUtils::DrawFullScreenQuad(
			finalBlitShader,
			mainWindow.GetMotionBlurTexture(1), "sceneTexture"
		);

		// ----- 7) HUD & Debug -----
		textShader.UseShader();
		fpsText.RenderText(
			textShader,
			std::string("FPS: ") + std::to_string(int(fps)),
			10.f, float(screenH - 30),
			0.4f, glm::vec3(1.f), orthoProj
		);
		
		fpsText.RenderText(
			textShader,
			std::string("Dog Health: ") + std::to_string(int(dogHealth)),
			10.f, float(screenH - 60),
			0.4f, glm::vec3(1.f), orthoProj
		);

		fpsText.RenderText(
			textShader,
			std::string("Player Health: ") + std::to_string(int(health)),
			10.f, float(screenH - 90),
			0.4f, glm::vec3(1.f), orthoProj
		);

		glm::mat4 camViewMatrix = camera.calculateViewMatrix();
		//for (auto& m : meshList)
		//	DrawBoundingBox(m->box, debugBox, perspProj, camViewMatrix);
		//for (auto& mdl : modelList)
		//	DrawBoundingBox(mdl->GetBox(), debugBox, perspProj, camViewMatrix);

		//healthBar.CreateSprite();
		healthBar.Render(spriteShader, orthoProj);

		handAnimation.Render(spriteShader, orthoProj);

		crosshairSprite.Render(spriteShader, orthoProj);

		rayRenderer.Render(perspProj, camViewMatrix, RayShader);

		// ----- 8) End frame -----
		glUseProgram(0);
		mainWindow.swapBuffers();
		updateFirstFrame(); // Update bounding boxes or triangles on first frame
		firstFrame = false;
	}
}
void Game::updateFirstFrame() {
	// On first frame, validate shadow shader
	if (firstFrame) {
		directionalShadowShader.Validate();
	}

	// Update bounding boxes for all meshes every frame
	for (auto& mesh : meshList) {
		mesh->CalculateModelSpaceBoundingBox();
		// Triangle collision: update on first frame or for non-rigid meshes thereafter
		if (!mesh->UsesBoxCollision) {
			if (firstFrame || !mesh->rigid) {
				mesh->UpdateTriangleList();
			}
		}
	}

	// Update bounding boxes for all models every frame
	for (auto& model : modelList) {
		model->CalculateModelSpaceBoundingBox();
		// Triangle collision: update on first frame or for non-rigid models thereafter
		if (!model->UsesBoxCollision) {
			if (firstFrame || !model->rigid) {
				model->UpdateTriangleList();
			}
		}
	}

	// Clear the firstFrame flag after initial setup
	firstFrame = false;
}
void Game::Update() {
	// Update camera position and other camera-related variables
	camera.previousPosition = camera.position;

	// Update deltaTime and FPS
	double now = glfwGetTime();
	deltaTime = now - deltaLastTime;
	deltaLastTime = now;

	frameCount++;

	if (now - fpsLastTime >= 1.0) {
		fps = static_cast<double>(frameCount) / (now - fpsLastTime);
		frameCount = 0;
		fpsLastTime = now;
	}


	// CHeck if dog is dead and update health bar
	if (dogHealth <= 0) {
		modelList[0]->IsValid = false;
		modelList[0]->ClearModel();
	}
	// Update Health Bar
	currentWidth = (health / 100.0f) * spriteWidth; // Scale health bar width based on health percentage
	rightX = centerX + currentWidth;

	// Update right side of the quad only
	healthBar.mVertices[4]  /* TR.x */ = rightX;
	healthBar.mVertices[8]  /* BR.x */ = rightX;


	// Call the Shadow Pass
	DirectionalShadowPass(&mainLight);
	// put into one list polymorpism
	for (size_t i = 0; i < pointLightCount; i++) {
		OmniShadowMapPass(&pointLights[i]);
	}
	for (size_t i = 0; i < spotLightCount; i++) {
		OmniShadowMapPass(&spotLights[i]);
	}


	
}
void Game::ProcessInput()
{
	// get handle user event inputs
	glfwPollEvents();
	camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange(), mainWindow.getYScrollChange(), mainWindow.getLeftClicked(), deltaTime);
	camera.keyControl(mainWindow.getKeys(), deltaTime);
	static bool keyHeld = false;
	if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_M) == GLFW_PRESS) {

	}

}
void Game::Update(float deltaTime)
{
}
void Game::RenderScene() {
	// BOX
	// translate first then move order matters
		// Update Bounding boxes if not rigid
	for (auto& mesh : meshList) {
		mesh->ResetModel(); // Reset model matrix
	}
	for (auto& model : modelList) {
		model->ResetModel(); // Reset model matrix
	}
	// ramp box
	//meshList[0]->translate(-2.0f, 4.5f, 9.0f);
	//meshList[0]->rotate(45, 0.0f, 0.0f, 1.0f);
	//meshList[0]->scale(10.0f, 2.0f, 1.0f);
	//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(meshList[0]->GetModel()));
	//tile.UseTexture();
	//shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	//meshList[0]->RenderMesh();

	Angle += 0.1f * deltaTime;
	if (Angle > 360.0f) {
		Angle = 0.0f;
	}

	// DOG CHASING CAMERA
	float dogSpeed = 1.0f; // Units per second (tweak as needed)

	glm::vec3 target = camera.getCameraPostion();  // Camera/player position

	// Optional: keep dog grounded by matching camera Y or fixed Y
	target.y -= camera.radiusY;
	glm::vec3 direction = target - dogPosition;
	float distance = glm::length(direction);

	// translate bounding box 
	if (distance > 0.1f) {  // Small threshold to stop jitter
		direction = glm::normalize(direction);
		dogPosition += direction * dogSpeed * deltaTime;
	}

	if (modelList[0]->IsValid) {

		modelList[0]->translate(dogPosition.x, dogPosition.y, dogPosition.z);

		glm::vec3 lookDir = glm::normalize(camera.getCameraPostion() - dogPosition);
		float angleY = atan2(lookDir.x, lookDir.z); // Yaw rotation to face player
		modelList[0]->rotate(angleY * toDegrees, 0.0, 1.0, 0.0);

		modelList[0]->scale(0.1, 0.1, 0.1);

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelList[0]->model));

		modelList[0]->RenderModel(uniformSpecularMap, uniformUseSpecularMap, uniformReflectivity, uniformUseReflectivity, uniformSkyBox, forestSkybox.GetTextureID());
	}

	int columns = 2;
	int rows = 2;
	int base = 1; // skip modelList[0]

	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < columns; ++col) {
			int idx = base + row * columns + col;
			if ((size_t)idx >= modelList.size()) break;  // safety check

			auto& box = modelList[idx];

			// reset transform every frame
			box->model = glm::mat4(1.0f);

			// scale & position in a grid
			box->scale(3.0f, 3.0f, 3.0f);
			box->translate(col * 2.0f, 0.0f, row * 2.0f);

			// draw
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(box->model));
			shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
			box->RenderModel(uniformSpecularMap, uniformUseSpecularMap, uniformReflectivity, uniformUseReflectivity, uniformSkyBox, forestSkybox.GetTextureID());
		}
	}

	glBindVertexArray(0);

}
void Game::CreateShaders() {

	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(shader1);

	directionalShadowShader = Shader();
	directionalShadowShader.CreateFromFiles("Shaders/directional_shadow_map_vert.glsl", "Shaders/directional_shadow_map_frag.glsl");

	directionalShadowShader.UseShader();
	uniformModel = directionalShadowShader.GetModelLocation();

	omniShadowShader = Shader();
	omniShadowShader.CreateFromFiles("Shaders/omni_shadow_map_vert.glsl", "Shaders/omni_shadow_map_geom.glsl", "Shaders/omni_shadow_map_frag.glsl");

	omniShadowShader.UseShader();

	uniformModel = omniShadowShader.GetModelLocation();
	uniformOmniLightPos = omniShadowShader.GetOmniLightPosLocation();
	uniformFarPlane = omniShadowShader.GetFarPlaneLocation();

	debugBox = Shader();
	debugBox.CreateFromFiles("Shaders/debug_box_vert.glsl", "Shaders/debug_box_frag.glsl");

	textShader = Shader();
	textShader.CreateFromFiles("Shaders/font_shader_vert.glsl", "Shaders/font_shader_frag.glsl");

	hudShader = Shader();
	hudShader.CreateFromFiles("Shaders/hud_vert.glsl", "Shaders/hud_frag.glsl");

	spriteShader = Shader();
	spriteShader.CreateFromFiles("Shaders/hud_vert.glsl", "Shaders/hud_frag.glsl");

	RayShader = Shader();
	RayShader.CreateFromFiles("Shaders/debug_line_vert.glsl", "Shaders/debug_line_frag.glsl");

	motionBlurShader = Shader();
	motionBlurShader.CreateFromFiles("Shaders/motion_blur_vert.glsl", "Shaders/motion_blur_frag.glsl");

	finalBlitShader = Shader();
	finalBlitShader.CreateFromFiles("Shaders/fullScreen_vert.glsl", "Shaders/fullScreen_frag.glsl");

	depthOfFieldShader = Shader();
	depthOfFieldShader.CreateFromFiles("Shaders/fullScreen_vert.glsl", "Shaders/depth_field_frag.glsl");

	shaderList[0]->UseShader();
	GLuint shaderID = shaderList[0]->GetShaderID();

	uniformEyePosition = shaderList[0]->GetEyePositionLocation();
	uniformSpecularIntensity = shaderList[0]->GetSpecularIntensityLocation();
	uniformShininess = shaderList[0]->GetShininessLocation();

	uniformSpecularMap = glGetUniformLocation(shaderID, "specularMap");
	uniformUseSpecularMap = glGetUniformLocation(shaderID, "useSpecularMap");

	uniformUseReflectivity = glGetUniformLocation(shaderID, "useReflectivity");
	uniformReflectivity = glGetUniformLocation(shaderID, "reflectivity");

	uniformSkyBox = glGetUniformLocation(shaderID, "skybox");
}
void Game::DirectionalShadowPass(DirectionalLight* light) {
	directionalShadowShader.UseShader();

	glViewport(0, 0, light->GetShadowMap()->GetShadowWidth(), light->GetShadowMap()->GetShadowHeight());

	light->GetShadowMap()->Write();
	glClear(GL_DEPTH_BUFFER_BIT);

	glm::mat4 lightTransform = light->CalculateLightTransform();
	directionalShadowShader.SetDirectionalLightTransform(&lightTransform);

	//directionalShadowShader.Validate();

	RenderScene();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Game::OmniShadowMapPass(PointLight* light) {
	omniShadowShader.UseShader();

	glViewport(0, 0, light->GetShadowMap()->GetShadowWidth(), light->GetShadowMap()->GetShadowHeight());

	light->GetShadowMap()->Write();
	glClear(GL_DEPTH_BUFFER_BIT);

	glUniform3f(uniformOmniLightPos, light->GetPosition().x, light->GetPosition().y, light->GetPosition().z);
	glUniform1f(uniformFarPlane, light->GetFarPlane());
	omniShadowShader.SetLightMatrices(light->CalculateLightTransform());

	omniShadowShader.Validate();

	RenderScene();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Game::RenderPass(glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {

	glViewport(0, 0, 1366, 768);
	// clear window
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//skybox.DrawSkybox(viewMatrix, projectionMatrix);
	forestSkybox.DrawSkybox(viewMatrix, projectionMatrix);

	shaderList[0]->UseShader();

	uniformModel = shaderList[0]->GetModelLocation();
	uniformProjection = shaderList[0]->GetProjectionLocation();
	uniformView = shaderList[0]->GetViewLocation();

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(perspProj));
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
void Game::scaleUVs(GLfloat* vertices, int totalFloatCount, int scale) {
	int floatsPerVertex = 8;

	int numVertices = totalFloatCount / floatsPerVertex;

	for (int i = 0; i < numVertices; ++i) {
		int offset = i * floatsPerVertex;

		// UVs are at index 3 and 4
		vertices[offset + 3] *= scale; // U
		vertices[offset + 4] *= scale; // V
	}
}
void Game::DrawBoundingBox(const BoundingBox& box, Shader& shader, const glm::mat4& projection, const glm::mat4& view)
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
	glDisable(GL_DEPTH_TEST);
	shader.UseShader();
	glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(shader.GetViewLocation(), 1, GL_FALSE, glm::value_ptr(view));

	// Draw
	glBindVertexArray(VAO);
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
}
void Game::Shoot()
{
	// Update cooldown timer
	if (shootCooldown > 0.0f) {
		shootCooldown -= deltaTime;
	}

	if (mainWindow.getLeftClicked() && shootCooldown <= 0.0f) {
		shootCooldown = shootCooldownDuration; // Reset cooldown

		handAnimation.Update(deltaTime); // Update hand animation on click
		gunAudioPlayer.Play();           // Play gun sound

		glm::vec3 rayOrigin = camera.getCameraPostion();
		glm::vec3 rayDirection = camera.getCameraDirection(); // normalized
		float maxDistance = 100.0f;

		rayRenderer.UpdateRay(rayOrigin, rayDirection, maxDistance);

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

		// Damage dog model if hit
		if (RayIntersectsAABB(rayOrigin, rayDirection, modelList[0]->GetBox(), maxDistance) && dogHealth > 0) {
			dogHealth -= 1.0f;
		}
	}
	else if (!mainWindow.getLeftClicked()) {
		handAnimation.Reset(); // Reset hand animation when not clicking
	}
}
void Game::CalcAverageNormals(unsigned int* indices, unsigned int indicieCount, GLfloat* vertices, unsigned int verticeCount, unsigned int vLength, unsigned int normalOffset) {
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
void Game::CheckCollision() {
	// MESH LIST OBJECT INTERSECTION CHECK
	for (const auto& mesh : meshList) {
		if (mesh->IsValid) {
			if (!mesh->UsesBoxCollision) {
				CheckTriangleCollision(closestY, hit, hitSide, hitTop, wallNormal, mesh, camera);
			}
			else {
				CheckBoxCollision(hit, hitSide, closestY, groundY, delta, mesh->box, true, camera, deltaTime);
			}
		}
	}

	if (modelList[0]->IsValid) {
		CheckBoxCollision(dogHit, hitSide, closestY, groundY, delta, modelList[0]->GetBox(), true, camera, deltaTime);
		if (dogHit && health > 0) {
			health -= 10.0f * deltaTime;
		}
	}
	// skip dog first model in list
	for (size_t i = 1; i < modelList.size(); ++i) {
		const auto& model = modelList[i];
		if (model->IsValid) {
			if (model->UsesBoxCollision) {
				model->CheckBoxCollisionModel(hit, hitSide, closestY, groundY, delta, true, camera, deltaTime);
			}
			else {
				model->CheckTriangleCollisionModel(closestY, hit, hitSide, hitTop, wallNormal, camera);
			}
		}
	}
}
void Game::GroundPlayer() {
	constexpr float groundSnapOffset = 0.01f;
	constexpr float maxGroundLevel = 0.0f;

	// Handle ground collision
	if (hit) {
		float feet = camera.position.y - camera.radiusY;
		float distanceToGround = feet - closestY;

		if (distanceToGround < 0.1f) {
			camera.isGrounded = true;
			camera.groundLevel = closestY;
			camera.position.y = closestY + camera.radiusY + groundSnapOffset;
			camera.verticalVelocity = 0.0f;
			return;
		}
	}

	// Fallback: use anyGrounded flag if no direct ground hit
	if (anyGrounded) {
		camera.isGrounded = true;
		camera.groundLevel = maxGroundLevel;
		camera.position.y = camera.groundLevel + camera.radiusY;
		camera.verticalVelocity = 0.0f;
	}
	else {
		camera.isGrounded = false;
		camera.groundLevel = 0.0f;
	}
	if (hitSide) {
		//std::cout << "Hit side wall!";
		// 1) how far you tried to move
		glm::vec3 moveDelta = camera.position - camera.previousPosition;

		// 2) rewind to last good spot
		camera.position = camera.previousPosition;

		// 3) pure plane‐projection (no epsilon)
		float d = glm::dot(moveDelta, wallNormal);
		glm::vec3 slideDelta = moveDelta - wallNormal * d;

		// 4) apply tangential movement
		camera.position += slideDelta;
	}
}