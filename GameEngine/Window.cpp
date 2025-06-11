#include "Window.h"
#include <iostream>
#include <vector>


Window::Window() {
	width = 800;
	height = 600;

	for (size_t i = 0; i < 1024; ++i) { keys[i] = 0; }

	xChange = 0.0f;
	yChange = 0.0f;

	leftMouseClicked = false;
}

Window::Window(GLint windowWidth, GLint windowHeight) {
	width = windowWidth;
	height = windowHeight;

	for (size_t i = 0; i < 1024; ++i) { keys[i] = 0; }

	xChange = 0.0f;
	yChange = 0.0f;

	yChangeScroll = 0.0f;

	leftMouseClicked = false;
}
int Window::Initialize() {
	//initialize glfw 
	if (!glfwInit()) {
		printf("GLFW Initialization failed!");
		glfwTerminate();
		return -1;
	}

	// setup window properties
	//opengl version

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// no backwards compatibility
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// allow forward compatibility
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//glfwWindowHint(GLFW_SAMPLES, 4);


	mainWindow = glfwCreateWindow(width, height, "Test Window", NULL, NULL);
	if (!mainWindow) {
		printf("GLFW window creation failed!");
		glfwTerminate();
		return 1;
	}

	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	// set context for GLEW to use 
	glfwMakeContextCurrent(mainWindow);


	// mouse and key input
	createCallbacks();
	glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Allow modern extension features
	glewExperimental = GL_TRUE;

	if (glewInit()) {
		printf("GLEW initialization failed!");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	//glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_CULL_FACE);  // safest if your geometry needs both faces visible


	glEnable(GL_MULTISAMPLE);
	// setup viewport size 
	glViewport(0, 0, bufferWidth, bufferHeight);

	glfwSetWindowUserPointer(mainWindow, this);

	for (int i = 0; i < 2; ++i) {
		// Create FBO
		glGenFramebuffers(1, &motionBlurFBO[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, motionBlurFBO[i]);

		// Create color texture attachment
		glGenTextures(1, &motionBlurTex[i]);
		glBindTexture(GL_TEXTURE_2D, motionBlurTex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, motionBlurTex[i], 0);

	
		glGenRenderbuffers(1, &motionBlurRBO[i]);
		glBindRenderbuffer(GL_RENDERBUFFER, motionBlurRBO[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, motionBlurRBO[i]);

		// Optional: check framebuffer completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Motion blur FBO " << i << " is not complete!\n";
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind
	}


	return 0;
}

GLfloat Window::getXChange()
{
	GLfloat theChange = xChange;
	xChange = 0.0f;
	return theChange;

}
GLfloat Window::getYChange()
{
	GLfloat theChange = yChange;
	yChange = 0.0f;
	return theChange;

}

GLfloat Window::getYScrollChange()
{
	GLfloat theChange = yChangeScroll;
	yChangeScroll = 0.0f;
	return theChange;
}


Window::~Window()
{
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
}

void Window::handleKeys(GLFWwindow* window, int key, int code, int action, int mode)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) {
			theWindow->keys[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			theWindow->keys[key] = false;
		}
	}
}

void Window::handleMouse(GLFWwindow* window, double xPos, double yPos)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (theWindow->mouseFirstMoved) {
		theWindow->lastX = xPos;
		theWindow->lastY = yPos;
		theWindow->mouseFirstMoved = false;
	}

	theWindow->xChange = xPos - theWindow->lastX;
	theWindow->yChange = theWindow->lastY - yPos;

	theWindow->lastX = xPos;
	theWindow->lastY = yPos;
}

void Window::handleScroll(GLFWwindow* window, double xOffset, double yOffset)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
	// Update the last scroll position
	theWindow->yChangeScroll = -1 * yOffset;

	//std::cout << "FOV Scroll Change: " << yOffset << std::endl;
}
void Window::handleMouseButtons(GLFWwindow* window, int button, int action, int mods)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		theWindow->leftMouseClicked = true;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		theWindow->leftMouseClicked = false;
	}
}

void Window::createCallbacks()
{
	glfwSetKeyCallback(mainWindow, handleKeys);
	glfwSetCursorPosCallback(mainWindow, handleMouse);
	glfwSetScrollCallback(mainWindow, handleScroll);
	glfwSetMouseButtonCallback(mainWindow, handleMouseButtons);
}
void Window::InitMotionBlurFBO(int width, int height) {
	// Create 2 ping-pong FBOs + textures + RBOs
	glGenFramebuffers(2, motionBlurFBO);
	glGenTextures(2, motionBlurTex);
	glGenRenderbuffers(2, motionBlurRBO);

	for (int i = 0; i < 2; i++) {
		// 1) Color texture
		glBindTexture(GL_TEXTURE_2D, motionBlurTex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// 2) Framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, motionBlurFBO[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D,
			motionBlurTex[i], 0);

		// Make sure we write to COLOR_ATTACHMENT0
		GLenum bufs[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, bufs);

		// 3) Depth+Stencil renderbuffer
		glBindRenderbuffer(GL_RENDERBUFFER, motionBlurRBO[i]);
		glRenderbufferStorage(GL_RENDERBUFFER,
			GL_DEPTH24_STENCIL8,
			width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
			GL_DEPTH_STENCIL_ATTACHMENT,
			GL_RENDERBUFFER,
			motionBlurRBO[i]);

		// 4) Check completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER)
			!= GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "Motion blur FBO[" << i << "] incomplete!\n";
		}
	}

	// 5) Unbind
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// 6) “Previous frame” texture (standalone)
	glGenTextures(1, &motionBlurPrevTex);
	glBindTexture(GL_TEXTURE_2D, motionBlurPrevTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// 7) Scene-color copy texture
	glGenTextures(1, &sceneColorTex);
	glBindTexture(GL_TEXTURE_2D, sceneColorTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, motionBlurFBO[1]);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


GLuint Window::GetMotionBlurFBO(int index) const {
	if (index < 0 || index > 1) {
		std::cerr << "[Window] Invalid FBO index: " << index << std::endl;
		return 0;
	}
	return motionBlurFBO[index];
}
GLuint Window::GetMotionBlurTexture(int index) const {
	if (index < 0 || index > 1) {
		std::cerr << "[Window] Invalid texture index: " << index << std::endl;
		return 0;
	}
	return motionBlurTex[index];
}
