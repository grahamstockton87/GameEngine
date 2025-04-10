#define STB_IMAGE_IMPLEMENTATION

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
#include "Camera.h"
#include "Texture.h"
#include "Light.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;

std::vector<Mesh*> meshList;

std::vector<Shader*> shaderList;

Camera camera;

Texture brickTexture;

Light mainLight;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

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
static const char* vShader = "Shaders/shaderVert.glsl";

// Fragment Shader
static const char* fShader = "Shaders/shaderFrag.glsl";
void CreateObjects()
{
    unsigned int indices2[]{
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    GLfloat vertices2[] = {
    //    x      y     z       u     v
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
        0.0f, -1.0f, 1.0f,    0.5f, 0.0f,
        1.0f, -1.0f, 0.0f,    1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,     0.5f, 1.0f
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
        // BOTTOM face (z = 0.0f)
        1.0f,  1.0f, 0.0f,   1.0f, 1.0f,   // top-right
       -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,   // top-left
       -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,   // bottom-left
        1.0f, -1.0f, 0.0f,   1.0f, 0.0f,   // bottom-right

        // TOP face (z = 1.0f)
        1.0f,  1.0f, 1.0f,   1.0f, 1.0f,   // top-right
       -1.0f,  1.0f, 1.0f,   0.0f, 1.0f,   // top-left
       -1.0f, -1.0f, 1.0f,   0.0f, 0.0f,   // bottom-left
        1.0f, -1.0f, 1.0f,   1.0f, 0.0f    // bottom-right
    };


    Mesh* cube = new Mesh();
    cube->CreateMesh(vertices, indices, 40, 36);
    meshList.push_back(cube);

    Mesh* pyramid = new Mesh();
    pyramid->CreateMesh(vertices2, indices2, 20, 12);
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

    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 5.0f, 10.0f, 5.0f);
    
    brickTexture = Texture("Textures/brick.png");
    brickTexture.LoadTexture();

    mainLight = Light();

    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformAmbientIntensity = 0, uniformAmbientColor = 0;
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

    // loop until know closed

    while (!mainWindow.getShouldClose()) {

        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;

        // get handle user event inputs
        glfwPollEvents();

        camera.keyControl(mainWindow.getKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange(), deltaTime);
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
        uniformView = shaderList[0]->GetViewLocation();
        uniformAmbientColor = shaderList[0]->GetAmbientColorLocation();
        uniformAmbientIntensity = shaderList[0]->GetAmbientIntensityLocation();

        mainLight.UseLight(uniformAmbientIntensity, uniformAmbientColor);

        // translate first then move order matters
        glm::mat4 model;
        //model = glm::translate(model, glm::vec3(triOffset, 0.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, -3.0f));
        model = glm::rotate(model, angle * toRadians, glm::vec3(1.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        brickTexture.UseTexture();
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