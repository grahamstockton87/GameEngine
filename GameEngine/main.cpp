#include <stdio.h>

#include <glew.h>
#include <GLFW/glfw3.h>
#include <mat4x4.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

#include "Mesh.h"
#include "Shader.h"
#include "Window.h"

double lastTime = 0.0;
double deltaTime = 0.0;

void update() {
    // Get current time
    double currentTime = glfwGetTime();

    // Calculate time difference (deltaTime)
    deltaTime = currentTime - lastTime;

    // Store current time as the last time for the next frame
    lastTime = currentTime;

    // Use deltaTime in your game loop for frame-rate independent movement or animation
}

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader*> shaderList;

bool direction = true;
float triOffset = 0.0f;
float triMaxOffset = 0.6f;
float triIncrement = 0.005f;

bool  sizeDirection = true;
float curSize = 0.4f;
float maxSize = 0.8f;
float minSize = 0.1f;

float angle = 0;

// Vertex Shader code
static const char* vShader = "Shaders/shader.vert";

// Fragment Shader
static const char* fShader = "Shaders/shader.frag";
void CreateObjects()
{
    unsigned int indices2[]{
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    GLfloat vertices2[] = {
        -1.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

    unsigned int indices[] = {
        4, 5, 6,  6, 7, 4, // TOP FACE
        0, 1, 2,  2, 3, 0, // Bottom Face
        1, 5, 4,  4, 0, 1, // RIGHT FACE
        6, 2, 1,  5, 6, 1, // BACK FACE
        6, 7, 3,  3, 2, 6, //LEFT FACE
        4, 7, 3,  3, 0, 4
    };



    GLfloat vertices[] = {
        // BOTTOM
        1.0f, 1.0f, 0.0f,
        -1.0f,1.0f, 0.0f,
        -1.0f,-1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,

        // TOP
        1.0f, 1.0f, 1.0f,
        -1.0f,1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        1.0f, -1.0f, 1.0f
    };

    Mesh* cube = new Mesh();
    cube->CreateMesh(vertices, indices, 24, 36);
    meshList.push_back(cube);

    Mesh* pyramid = new Mesh();
    pyramid->CreateMesh(vertices2, indices2, 24, 36);
    meshList.push_back(pyramid);

}


void CreateShaders() {
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(shader1);
}

int main() {
    
    mainWindow = Window(800, 600);
    mainWindow.Initialize();
    CreateObjects();
    CreateShaders();

    GLuint uniformProjection = 0, uniformModel = 0;
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

    // loop until know closed

    while (!mainWindow.getShouldClose()) {

        // get handle user event inputs
        glfwPollEvents();

        //update();

        if (direction) {
            triOffset += triIncrement;
        }
        else {
            triOffset -= triIncrement;
        }

        if (abs(triOffset) >= triMaxOffset) {
            direction = !direction;
        }
        if (sizeDirection) {
            curSize += 0.001f;
        }
        else {
            curSize -= 0.001f;
        }
        if (curSize >= maxSize || curSize <= minSize) {
            sizeDirection = !sizeDirection;
        }
        if (angle < 360) {
            angle += 1;
        }
        else {
            angle = 0;
        }
        // clear window

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderList[0]->UseShader();
        uniformModel = shaderList[0]->GetModelLocation();
        uniformProjection = shaderList[0]->GetProjectionLocation();

        // translate first then move order matters
        glm::mat4 model;
        //model = glm::translate(model, glm::vec3(triOffset, 0.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, -3.0f));
        model = glm::rotate(model, angle * toRadians, glm::vec3(1.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        meshList[0]->RenderMesh();

        model = glm::mat4();
        model = glm::translate(model, glm::vec3(0.0f, 1.0f, -3.0f));
        model = glm::rotate(model, angle * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        meshList[1]->RenderMesh();


        glBindVertexArray(0);

        glUseProgram(0);

        mainWindow.swapBuffers();
    }
    return 0;
}