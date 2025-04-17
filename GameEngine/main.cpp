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
#include "CommonValues.h"

#include "Mesh.h"
#include "Shader.h"
#include "Window.h"
#include "Camera.h"
#include "Texture.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;

std::vector<Mesh*> meshList;

std::vector<Shader*> shaderList;

Camera camera;

Texture brickTexture;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];

Material shinyMaterial;
Material dullMaterial;

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

void calcAverageNormals(unsigned int* indices, unsigned int indicieCount, GLfloat* vertices, 
                        unsigned int verticeCount, unsigned int vLength, unsigned int normalOffset) {
    for (size_t i = 0; i < indicieCount; i += 3) {
        unsigned int in0 = indices[i] * vLength;
        unsigned int in1 = indices[i+1] * vLength;
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
        vertices[nOffset] = vec.x; vertices[nOffset+1] = vec.y; vertices[nOffset+2] = vec.z;
    }
}


void CreateObjects()
{
    unsigned int indices2[]{
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    GLfloat vertices2[] = {
    //    x      y     z       u     v        N
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,    0.0f,0.0f,0.0f,
        0.0f, -1.0f, 1.0f,    0.5f, 0.0f,    0.0f,0.0f,0.0f,
        1.0f, -1.0f, 0.0f,    1.0f, 0.0f,    0.0f,0.0f,0.0f,
        0.0f, 1.0f, 0.0f,     0.5f, 1.0f,    0.0f,0.0f,0.0f
    };

    calcAverageNormals(indices2, 12, vertices2, 32, 8, 5);

    unsigned int indices[] = {
        4, 5, 6,  6, 7, 4, // TOP FACE
        0, 1, 2,  2, 3, 0, // Bottom Face
        1, 5, 4,  4, 0, 1, // RIGHT FACE
        6, 2, 1,  5, 6, 1, // BACK FACE
        6, 7, 3,  3, 2, 6, //LEFT FACE
        4, 7, 3,  3, 0, 4
    };

    
    GLfloat vertices[] = {
      // x      y       z       u    v         nx    ny    nz
        1.0f,  1.0f, -1.0f,   1.0f, 1.0f,    0.0f, 0.0f, 0.0f, // top-right
       -1.0f,  1.0f, -1.0f,   0.0f, 1.0f,    0.0f, 0.0f, 0.0f, // top-left
       -1.0f, -1.0f, -1.0f,   0.0f, 0.0f,    0.0f, 0.0f, 0.0f, // bottom-left
        1.0f, -1.0f, -1.0f,   1.0f, 0.0f,    0.0f, 0.0f, 0.0f, // bottom-right

        // TOP face (z = 1.0f)
        1.0f,  1.0f, 1.0f,   1.0f, 1.0f,    0.0f, 0.0f, 0.0f, // top-right
       -1.0f,  1.0f, 1.0f,   0.0f, 1.0f,    0.0f, 0.0f, 0.0f, // top-left
       -1.0f, -1.0f, 1.0f,   0.0f, 0.0f,    0.0f, 0.0f, 0.0f, // bottom-left
        1.0f, -1.0f, 1.0f,   1.0f, 0.0f,    0.0f, 0.0f, 0.0f  // bottom-right
    };

    calcAverageNormals(indices, 36, vertices, 64, 8, 5);

    unsigned int floorIndices[]{
        0, 2, 1,
        1, 2, 3
    };

    GLfloat floorVertices[] = {
        -10.0f, 0.0f, -10.0f,   0.0f, 0.0f,     0.0f, -1.0f, 0.0f,
        10.0f, 0.0f, -10.0f,    10.0f, 0.0f,    0.0f, -1.0f, 0.0f,
        -10.0f, 0.0f, 10.0f,    0.0f, 10.0f,    0.0f, -1.0f, 0.0f,
        10.0f, 0.0f, 10.0f,     10.0f, 10.0f,   0.0f, -1.0f, 0.0f
    };

    

    Mesh* cube = new Mesh();
    cube->CreateMesh(vertices, indices, 64, 36);
    meshList.push_back(cube);

    Mesh* pyramid = new Mesh();
    pyramid->CreateMesh(vertices2, indices2, 32, 12);
    meshList.push_back(pyramid);

    Mesh* floor = new Mesh();
    floor->CreateMesh(floorVertices, floorIndices, 32, 6);
    meshList.push_back(floor);

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

    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 5.0f, 10.0f, 5.0f, 45.0f);
    
    shinyMaterial = Material(1.0f, 128);
    dullMaterial = Material(0.3f, 4);

    brickTexture = Texture("Textures/brick.png");
    brickTexture.LoadTexture();

    mainLight = DirectionalLight(1.0f, 1.0f, 1.0f, 
                                 0.0f, 0.0f, 
                                 2.0f, -1.0f, -2.0f);
    unsigned int pointLightCount = 0;
    pointLights[0] = PointLight(1.0f, 0.0f, 0.0f,
                                0.0f, 1.0f,
                                -4.0f, 2.0f, 0.0f,
                                0.3f, 0.1f, 0.1f);
    pointLightCount++;
    pointLights[1] = PointLight(0.0f, 0.0f, 1.0f,
                                0.0f, 1.0f,
                                4.0f, 2.0f, 0.0f,
                                0.3f, 0.1f, 0.1f);
    pointLightCount++;
  

    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0, uniformSpecularIntensity = 0, uniformShininess = 0, uniformDirection = 0, uniformDiffuseIntensity = 0;
    

    // loop until know closed

    while (!mainWindow.getShouldClose()) {

       

        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;

        // get handle user event inputs
        glfwPollEvents();

        camera.keyControl(mainWindow.getKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange(), mainWindow.getYScrollChange(), deltaTime);
        //update();
        glm::mat4 projection = glm::perspective(camera.getFov(), (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

        //std::cout << camera.getFov() << std::endl;
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
        uniformEyePosition = shaderList[0]->GetEyePositionLocation();
        uniformSpecularIntensity = shaderList[0]->GetSpecularIntensityLocation();
        uniformShininess = shaderList[0]->GetShininessLocation();

        //shaderList[0]->SetDirectionalLight(&mainLight);
        shaderList[0]->SetPointLights(pointLights, pointLightCount);
      
        // translate first then move order matters
        glm::mat4 model;
        //model = glm::translate(model, glm::vec3(triOffset, 0.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, -3.0f));
        model = glm::rotate(model, 0 * toRadians, glm::vec3(1.0f, 1.0f, 0.0f));
        //model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        glUniform3f(uniformEyePosition, camera.getCameraPostion().x, camera.getCameraPostion().y, camera.getCameraPostion().z);
        brickTexture.UseTexture();
        shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[0]->RenderMesh();

        model = glm::mat4();
        model = glm::translate(model, glm::vec3(0.0f, 1.0f, -3.0f));
        model = glm::rotate(model, 0 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        //model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[1]->RenderMesh();

        model = glm::mat4();
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        //model = glm::rotate(model, 0 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        //model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[2]->RenderMesh();


        glBindVertexArray(0);

        glUseProgram(0);

        mainWindow.swapBuffers();
    }
    return 0;
}