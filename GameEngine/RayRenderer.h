#pragma once

#include <glm.hpp>
#include <GL/glew.h>

class RayRenderer {
public:
    RayRenderer();
    ~RayRenderer();

    void Init();                      // Sets up VAO and VBO
    void UpdateRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance);
    void Render();                   // Optional if you want to draw it

private:
    GLuint rayVAO = 0;
    GLuint rayVBO = 0;
};
