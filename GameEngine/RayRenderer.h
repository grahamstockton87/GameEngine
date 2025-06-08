#pragma once

#include <GL/glew.h>
#include <glm.hpp>

#include "Shader.h"

class RayRenderer {
public:
    RayRenderer();  // Default constructor
    ~RayRenderer();

    void Init();                      // Sets up VAO and VBO
    void UpdateRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance);
    void Render(const glm::mat4& projection, const glm::mat4& view, Shader& rayShader);

private:
    GLuint rayVAO = 0;
    GLuint rayVBO = 0;

    bool initialized = false;  // NEW: track whether buffers are created
};
