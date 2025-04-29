#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <mat4x4.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

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
#include "Model.h"

const float toRadians = 3.14159265f / 180.0f;

// lights class list
unsigned int pointLightCount = 0;
unsigned int spotLightCount = 0;

GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0, uniformSpecularIntensity = 0, uniformShininess = 0, uniformDirection = 0, uniformDiffuseIntensity = 0;

Window mainWindow;

Camera camera(glm::vec3(10.0f, 5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 5.0f, 100.0f, 5.0f, 45.0f);

std::vector<Mesh*> meshList;

std::vector<Shader*> shaderList;
Shader directionalShadowShader;


Texture brickTexture;
Texture transparent;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

Material shinyMaterial;
Material dullMaterial;

Model dog;

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


void CreateShaders() {
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(shader1);

    directionalShadowShader = Shader();
    directionalShadowShader.CreateFromFiles("Shaders/directional_shadow_map_vert.glsl", "Shaders/directional_shadow_map_frag.glsl");
}
void RenderScene() {
    // BOX
    // translate first then move order matters

    glm::mat4 model;
    model = glm::translate(model, glm::vec3(0.0f, 5.0f, 0.0f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    brickTexture.UseTexture();
    shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
    meshList[0]->RenderMesh();

    //camera.boxCollision(meshList[0]->box);

    // PYRAMID
    model = glm::mat4();
    model = glm::translate(model, glm::vec3(0.0f, 3.0f, -3.0f));
    model = glm::rotate(model, 0 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
    meshList[1]->RenderMesh();

    // FLOOR
    model = glm::mat4();
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
    meshList[2]->RenderMesh();

    Angle += 0.1f * deltaTime;
    if (Angle > 360.0f) {
        Angle = 0.0f;
    }
    // DOG
    model = glm::mat4();
    model = glm::rotate(model, Angle, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-5.0f, 1.0f, 0.0f));
    model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
    dog.RenderModel();

    glBindVertexArray(0);
}
void DirectionalShadowPass(DirectionalLight* light) {
    directionalShadowShader.UseShader();

    glViewport(0, 0, light->GetShadowMap()->GetShadowWidth(), light->GetShadowMap()->GetShadowHeight());

    light->GetShadowMap()->Write();
    glClear(GL_DEPTH_BUFFER_BIT);

    uniformModel = directionalShadowShader.GetModelLocation();
    glm::mat4 lightTransform = light->CalculateLightTransform();
    directionalShadowShader.SetDirectionalLightTransform(&lightTransform);


    RenderScene();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void RenderPass(glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {
    shaderList[0]->UseShader();
    uniformModel = shaderList[0]->GetModelLocation();
    uniformProjection = shaderList[0]->GetProjectionLocation();
    uniformView = shaderList[0]->GetViewLocation();
    uniformEyePosition = shaderList[0]->GetEyePositionLocation();
    uniformSpecularIntensity = shaderList[0]->GetSpecularIntensityLocation();
    uniformShininess = shaderList[0]->GetShininessLocation();

    glViewport(0, 0, 1366, 768);

    // clear window
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
    glUniform3f(uniformEyePosition, camera.getCameraPostion().x, camera.getCameraPostion().y, camera.getCameraPostion().z);

    shaderList[0]->SetDirectionalLight(&mainLight);
    shaderList[0]->SetPointLights(pointLights, pointLightCount);
    shaderList[0]->SetSpotLights(spotLights, spotLightCount);
    glm::mat4 lightTransform = mainLight.CalculateLightTransform();
    shaderList[0]->SetDirectionalLightTransform(&lightTransform);

    mainLight.GetShadowMap()->Read(GL_TEXTURE1);
    shaderList[0]->SetTexture(0);
    shaderList[0]->SetDirectionalShadowMap(1);
   
    glm::vec3 lowerLight = camera.getCameraPostion();
    lowerLight.y -= 0.1f;
    spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

    RenderScene();

}

int main() {
    
    mainWindow = Window(1200, 800);
    mainWindow.Initialize();

// OBJECTS --------------------------------------------------------------------------------
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




    GLfloat vertices[] = {
        // FRONT (+Z)
        -1, -1,  1,   0, 0,   0,  0, -1,
         1, -1,  1,   1, 0,   0,  0, -1,
         1,  1,  1,   1, 1,   0,  0, -1,
        -1,  1,  1,   0, 1,   0,  0, -1,

        // BACK (-Z)
         1, -1, -1,   0, 0,   0,  0, 1,
        -1, -1, -1,   1, 0,   0,  0, 1,
        -1,  1, -1,   1, 1,   0,  0, 1,
         1,  1, -1,   0, 1,   0,  0, 1,

         // LEFT (-X)
         -1, -1, -1,   0, 0,   1,  0,  0,
         -1, -1,  1,   1, 0,   1,  0,  0,
         -1,  1,  1,   1, 1,   1,  0,  0,
         -1,  1, -1,   0, 1,   1,  0,  0,

         // RIGHT (+X)
          1, -1,  1,   0, 0,  -1,  0,  0,
          1, -1, -1,   1, 0,  -1,  0,  0,
          1,  1, -1,   1, 1,  -1,  0,  0,
          1,  1,  1,   0, 1,  -1,  0,  0,

          // TOP (+Y)
          -1,  1,  1,   0, 0,   0, -1,  0,
           1,  1,  1,   1, 0,   0, -1,  0,
           1,  1, -1,   1, 1,   0, -1,  0,
          -1,  1, -1,   0, 1,   0, -1,  0,

          // BOTTOM (-Y)
          -1, -1, -1,   0, 0,   0, 1,  0,
           1, -1, -1,   1, 0,   0, 1,  0,
           1, -1,  1,   1, 1,   0, 1,  0,
          -1, -1,  1,   0, 1,   0, 1,  0
    };



    //calcAverageNormals(indices, 36, vertices, 192, 8, 5);

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


    Mesh* cube = new Mesh(vertices, indices, 192, 36);
    cube->CreateMesh();
    meshList.push_back(cube);

    Mesh* pyramid = new Mesh(vertices2, indices2, 32, 12);
    pyramid->CreateMesh();
    meshList.push_back(pyramid);

    Mesh* floor = new Mesh(floorVertices, floorIndices, 32, 6);
    floor->CreateMesh();
    meshList.push_back(floor);

    CreateShaders();
    
    // MATERIALS  -------------------------------------------------------------------------------------------
    shinyMaterial = Material(1.0f, 128);
    dullMaterial = Material(0.3f, 4);

    // TEXTURES --------------------------------------------------------------------------------------------
    brickTexture = Texture("Textures/brick.png");
    brickTexture.LoadTextureA();

    transparent = Texture("Textures/transparent.png");
    transparent.LoadTextureA();

    dog = Model();
    dog.LoadModel("Models/dog.obj");

    // LIGHTS --------------------------------------------------------------------------------------------
    mainLight = DirectionalLight(1024, 1024,
                                1.0f, 1.0f, 1.0f, 
                                 0.1f, 0.6f, 
                                 0.0f, -7.0f, -1.0f);

    pointLights[0] = PointLight(1024, 1024,
                                1.0f, 0.0f, 0.0f,
                                0.0f, 1.0f,
                                -4.0f, 2.0f, 0.0f,
                                0.3f, 0.1f, 0.1f);
    pointLightCount++;
    pointLights[1] = PointLight(1024, 1024, 
                                0.0f, 0.0f, 1.0f,
                                0.0f, 1.0f,
                                4.0f, 2.0f, 0.0f,
                                0.3f, 0.1f, 0.1f);
    pointLightCount++;

    
    spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f, // color
                              0.0f, 5.0f, // ambient diffuse
                              4.0f, 1.0f, 0.0f, // position
                              0.0f, -1.0f, 0.0f, // direction
                              0.3f, 0.1f, 0.1f, // Equation
                              40.0f); // Angle
    spotLightCount++;
    
    Assimp::Importer importer;
    // loop until know closed

    while (!mainWindow.getShouldClose()) {

       

        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;

        // get handle user event inputs
        glfwPollEvents();

  
        //for (Mesh* mesh: meshList) {
        //    camera.boxCollision(mesh->box);
        //}
        
        //std::cout << "Min = " << meshList[0]->box.min.x << meshList[0]->box.min.y << meshList[0]->box.min.z << std::endl;
        //std::cout << "Max = " << meshList[0]->box.max.x << meshList[0]->box.max.y << meshList[0]->box.max.z << std::endl;

        //update();
        glm::mat4 projection = glm::perspective(camera.getFov(), (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

        //std::cout << camera.getFov() << std::endl;
        //if (direction) {
        //    triOffset += triIncrement;
        //}
        //else {
        //    triOffset -= triIncrement;
        //}

        //if (abs(triOffset) >= triMaxOffset) {
        //    direction = !direction;
        //}
        //if (sizeDirection) {
        //    curSize += 0.001f;
        //}
        //else {
        //    curSize -= 0.001f;
        //}
        //if (curSize >= maxSize || curSize <= minSize) {
        //    sizeDirection = !sizeDirection;
        //}
        //if (angle < 360) {
        //    angle += 1;
        //}
        //else {
        //    angle = 0;
        //}
        // 
      
        // clear window
        camera.keyControl(mainWindow.getKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange(), mainWindow.getYScrollChange(), deltaTime);
        camera.updatePhysics(deltaTime);

        DirectionalShadowPass(&mainLight);
        RenderPass(projection, camera.calculateViewMatrix());

        glUseProgram(0);

        mainWindow.swapBuffers();
    }
    return 0;
}