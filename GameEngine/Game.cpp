#include "Game.h"
Game::Game()
{
}
Game::~Game()
{
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

	transparent = Texture("Textures/transparent.png");
	transparent.LoadTextureA();

	tile = Texture("Textures/tile.png");
	tile.LoadTexture();

	wallpaper = Texture("Textures/wallpaper.jpg");
	wallpaper.LoadTexture();

	window = Texture("Textures/window2.png");
	window.LoadTextureA();

	healthBarTexture = Texture("Textures/healthBar.jpg");
	healthBarTexture.LoadTexture();

	dog = std::make_unique<Model>();
	dog->LoadModel("Models/dog.obj");
	dog->rigid = false;
	modelList.push_back(std::move(dog));

	land = std::make_unique<Model>();
	land->LoadModel("Models/LevelDesignBase2.obj");
	land->scaleUVs(20.0f);
	land->model = glm::mat4(1.0f);
	land->scale(3.0f, 3.0f, 3.0f);
	land->SetRigid(true);
	land->UsesBoxCollision = false; // Disable box collision for this model
	modelList.push_back(std::move(land));
	
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

	GLfloat verticesSprite[] = {
		// positions									// texCoords
		xDisplacement + xCenter - spriteWidth / 2, yTop,				0.0f, 0.0f,
		xDisplacement + xCenter + spriteWidth / 2, yTop,				1.0f, 0.0f,
		xDisplacement + xCenter + spriteWidth / 2, yTop - spriteHeight, 1.0f, 1.0f,
		xDisplacement + xCenter - spriteWidth / 2, yTop - spriteHeight, 0.0f, 1.0f
	};
	unsigned int indicesSprite[] = { 0, 1, 2, 2, 3, 0 };

	healthBar = Sprite(verticesSprite, indicesSprite, 16, 6);
	healthBar.CreateSprite();

	healthHUD.Initialize(xCenter, yTop, 400.0f, 20.0f, xDisplacement);
	healthHUD.SetTexture(&healthBarTexture);

	std::unique_ptr<Mesh> ramp = std::make_unique<Mesh>(boxVertices, boxIndices, 192, 36);
	ramp->CreateMesh();
	ramp->ID = "ramp";
	ramp->UsesBoxCollision = false;
	meshList.push_back(std::move(ramp));

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

	skybox = Skybox(skyBoxFaces);

	Assimp::Importer importer;
	// loop until know closed
	AudioPlayer player;

	if (!player.LoadMP3("audio/test.mp3")) {
		return -1;
	}

	player.Play();
	std::cout << "Playing MP3..." << std::endl;

	mainWindow.InitMotionBlurFBO(mainWindow.getBufferWidth(), mainWindow.getBufferHeight());

}

void Game::Run() {
	// Figure out which FBO is scene and which holds history:
	const GLuint sceneFBO = mainWindow.GetMotionBlurFBO(0);
	const GLuint historyFBO = mainWindow.GetMotionBlurFBO(1);

	bool firstFrame = true;

	// -- One-time clear so history starts empty --
	for (GLuint fbo : { sceneFBO, historyFBO }) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	// Main loop
	while (!mainWindow.getShouldClose()) {
		// ----- 1) Update game state -----
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
		glm::mat4 orthoProj = glm::ortho(
			0.0f, float(mainWindow.getBufferWidth()),
			0.0f, float(mainWindow.getBufferHeight())
		);
		glm::mat4 perspProj = glm::perspective(
			glm::radians(camera.getFov()),
			float(mainWindow.getBufferWidth()) / mainWindow.getBufferHeight(),
			0.1f, 100.0f
		);

		// ----- 3) Render scene into sceneFBO -----
		glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
		glViewport(0, 0,
			mainWindow.getBufferWidth(),
			mainWindow.getBufferHeight());
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderPass(perspProj, camera.calculateViewMatrix());

		// ----- 4) Blend scene + history into historyFBO -----
		glBindFramebuffer(GL_FRAMEBUFFER, historyFBO);
		glViewport(0, 0, mainWindow.getBufferWidth(), mainWindow.getBufferHeight());
		glDisable(GL_DEPTH_TEST);
		// clear out only the color (we’ll draw fresh)
		//glClear(GL_COLOR_BUFFER_BIT);

		motionBlurShader.UseShader();
		RenderUtils::DrawFullScreenQuad(
			motionBlurShader,
			// unit0 = new scene
			mainWindow.GetMotionBlurTexture(0), "currentFrame",
			// unit1 = old history (or scene on first frame)
			firstFrame
			? mainWindow.GetMotionBlurTexture(0)
			: mainWindow.GetMotionBlurTexture(1),
			"previousFrame",
			// blend 90% new + 10% old
			0.9f
		);

		// ----- 5) Blit accumulated result to screen -----
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		finalBlitShader.UseShader();
		RenderUtils::DrawFullScreenQuad(
			finalBlitShader,
			mainWindow.GetMotionBlurTexture(1), "sceneTexture"
		);

		// ----- 6) HUD & debug overlays -----
		textShader.UseShader();
		std::string fpsValue = "FPS: " + std::to_string(int(fps));
		fpsText.RenderText(
			textShader, fpsValue,
			10.0f, mainWindow.getBufferHeight() - 30.0f,
			0.4f, glm::vec3(1.0f), orthoProj
		);

		debugBox.UseShader();
		for (auto& m : meshList)
			DrawBoundingBox(m->box, debugBox, perspProj, camera.calculateViewMatrix());
		for (auto& mdl : modelList)
			DrawBoundingBox(mdl->GetBox(), debugBox, perspProj, camera.calculateViewMatrix());

		healthBar.CreateSprite();
		healthHUD.Render(hudShader, orthoProj);
		rayRenderer.Render(perspProj, camera.calculateViewMatrix(), RayShader);

		// ----- 7) Finish frame -----
		glUseProgram(0);
		mainWindow.swapBuffers();
		firstFrame = false;
	}
}








//if (debugDisplayMode == 0) {
//	glBindTexture(GL_TEXTURE_2D, mainWindow.GetMotionBlurTexture(nextFBO));
//	std::cout << "[BLEND] nextFBO\n";
//}
//else if (debugDisplayMode == 1) {
//	glBindTexture(GL_TEXTURE_2D, mainWindow.motionBlurPrevTex);
//	std::cout << "[PREV] motionBlurPrevTex\n";
//}
//else {
//	glBindTexture(GL_TEXTURE_2D, mainWindow.GetMotionBlurTexture(currentFBO));
//	std::cout << "[CURR] currentFBO\n";
//}



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
	currentWidth = (health / 100.0f) * spriteWidth; // Scale health bar width based on health percentage
	rightX = centerX + currentWidth;

	// Update right side of the quad only
	healthBar.mVertices[4] = rightX;  // bottom-right x
	healthBar.mVertices[8] = rightX;  // top-right x

	// Call the Shadow Pass
	DirectionalShadowPass(&mainLight);
	// put into one list polymorpism
	for (size_t i = 0; i < pointLightCount; i++) {
		OmniShadowMapPass(&pointLights[i]);
	}
	for (size_t i = 0; i < spotLightCount; i++) {
		OmniShadowMapPass(&spotLights[i]);
	}
	// Update Bounding Boxes or Triangles
	for (auto& mesh : meshList) {
		if (!mesh->rigid || firstFrame) {
			mesh->CalculateModelSpaceBoundingBox(); // Calculate bounding box
		}
	}
	for (auto& model : modelList) {
		if (!model->rigid || firstFrame) {
			model->CalculateModelSpaceBoundingBox(); // Calculate bounding box
		}
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
		
		if (!keyHeld) {
			debugDisplayMode = (debugDisplayMode + 1) % 3;
			std::cout << "Debug display mode: " << debugDisplayMode << std::endl;
			keyHeld = true;
		}
	}
	else {
		keyHeld = false;
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
	meshList[0]->translate(-2.0f, 4.5f, 9.0f);
	meshList[0]->rotate(45, 0.0f, 0.0f, 1.0f);
	meshList[0]->scale(10.0f, 2.0f, 1.0f);
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(meshList[0]->GetModel()));
	tile.UseTexture();
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();

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

		shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		modelList[0]->RenderModel(uniformModel);
	}

	// ROOM
	modelList[1]->scale(3.0f, 3.0f, 3.0f);
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelList[1]->model));
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	modelList[1]->RenderModel(uniformModel, false);

	glBindVertexArray(0);

}
void Game::CreateShaders() {
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(shader1);

	directionalShadowShader = Shader();
	directionalShadowShader.CreateFromFiles("Shaders/directional_shadow_map_vert.glsl", "Shaders/directional_shadow_map_frag.glsl");

	omniShadowShader = Shader();
	omniShadowShader.CreateFromFiles("Shaders/omni_shadow_map_vert.glsl", "Shaders/omni_shadow_map_geom.glsl", "Shaders/omni_shadow_map_frag.glsl");

	debugBox = Shader();
	debugBox.CreateFromFiles("Shaders/debug_box_vert.glsl", "Shaders/debug_box_frag.glsl");

	textShader = Shader();
	textShader.CreateFromFiles("Shaders/font_shader_vert.glsl", "Shaders/font_shader_frag.glsl");

	hudShader = Shader();
	hudShader.CreateFromFiles("Shaders/hud_vert.glsl", "Shaders/hud_frag.glsl");

	RayShader = Shader();
	RayShader.CreateFromFiles("Shaders/debug_line_vert.glsl", "Shaders/debug_line_frag.glsl");

	motionBlurShader = Shader();
	motionBlurShader.CreateFromFiles("Shaders/motion_blur_vert.glsl", "Shaders/motion_blur_frag.glsl");

	finalBlitShader = Shader();
	finalBlitShader.CreateFromFiles("Shaders/fullScreen_vert.glsl", "Shaders/fullScreen_frag.glsl");
}
void Game::DirectionalShadowPass(DirectionalLight* light) {
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
void Game::OmniShadowMapPass(PointLight* light) {
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
void Game::RenderPass(glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {

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
	shader.UseShader();
	glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(shader.GetViewLocation(), 1, GL_FALSE, glm::value_ptr(view));

	// Draw
	glBindVertexArray(VAO);
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
void Game::Shoot()
{
	// SHOOT
	if (mainWindow.getLeftClicked()) {

		glm::vec3 rayOrigin = camera.getCameraPostion();
		glm::vec3 rayDirection = camera.getCameraDirection(); // normalized
		float maxDistance = 4.0f;

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
		// Ray hit test...
		if (RayIntersectsAABB(rayOrigin, rayDirection, modelList[0]->GetBox(), maxDistance)){
			dogHealth -= 100.0f;
		}
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

	for (size_t i = 1; i < modelList.size(); ++i) {
		const auto& model = modelList[i];

		if (model->IsValid && model->UsesBoxCollision) {
			model->CheckBoxCollisionModel(hit, hitSide, closestY, groundY, delta, true, camera, deltaTime);
		}
		else {
			model->CheckTriangleCollisionModel(closestY, hit, hitSide, hitTop, wallNormal, camera);
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
	// Handle side collision and wall sliding
	if (hitSide && glm::length(wallNormal) > 0.0f) {
		glm::vec3 moveDelta = camera.position - camera.previousPosition;
		glm::vec3 slideVector = ProjectMovementOntoWall(moveDelta, wallNormal);
		camera.position = camera.previousPosition + slideVector;
	}
}