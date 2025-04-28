#pragma once

#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window {
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);

	int Initialize();

	GLint getBufferWidth() { return bufferWidth;}
	GLint getBufferHeight() { return bufferHeight; }

	bool getShouldClose() { return glfwWindowShouldClose(mainWindow); }

	bool* getKeys(){return keys;}
	GLfloat getXChange();
	GLfloat getYChange();
	GLfloat getYScrollChange();

	void swapBuffers() { glfwSwapBuffers(mainWindow); }

	~Window();
private:
	GLFWwindow* mainWindow;
	GLint width, height;
	GLint bufferWidth, bufferHeight;

	bool keys[1024];

	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	GLfloat lastXScroll;
	GLfloat lastYScroll;
	GLfloat xChangeScroll;
	GLfloat yChangeScroll;

	bool mouseFirstMoved;
	bool wheelFirstMoved;

	static void handleKeys(GLFWwindow* window, int key, int code, int action, int mode);
	static void handleMouse(GLFWwindow* window, double xPos, double yPos);
	static void handleScroll(GLFWwindow* window, double xOffset, double yOffset);

	void createCallbacks();
};

