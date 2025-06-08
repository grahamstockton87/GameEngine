#include "RayRenderer.h"

RayRenderer::RayRenderer() {
    rayVAO = 0;
	rayVBO = 0;

    // You MUST call Init() manually if you use this constructor
}
RayRenderer::~RayRenderer() {
    if (rayVAO) glDeleteVertexArrays(1, &rayVAO);
    if (rayVBO) glDeleteBuffers(1, &rayVBO);
}

void RayRenderer::Init() {
    if (initialized) return;  // prevent double init

    glGenVertexArrays(1, &rayVAO);
    glGenBuffers(1, &rayVBO);

    glBindVertexArray(rayVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rayVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    initialized = true;
}

void RayRenderer::UpdateRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) {
    if (!initialized) return;  // avoid crash
    glm::vec3 rayStart = origin;
    glm::vec3 rayEnd = origin + direction * maxDistance;
    glm::vec3 rayVerts[2] = { rayStart, rayEnd };

    glBindBuffer(GL_ARRAY_BUFFER, rayVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(rayVerts), rayVerts);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void RayRenderer::Render(const glm::mat4& projection, const glm::mat4& view, Shader& rayShader) {
    if (!initialized || rayVAO == 0 || rayShader.GetShaderID() == 0) {
        std::cerr << "[RayRenderer::Render] Skipping draw due to: "
            << "initialized = " << (initialized) << ", "
            << "rayVAO = " << rayVAO << ", "
            << "shader ID = " << rayShader.GetShaderID()
            << std::endl;
        return;
    }
	rayShader.UseShader();

    // DEBUG LINE RAY
   
    glLineWidth(10.0f);

    glUniformMatrix4fv(rayShader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(rayShader.GetViewLocation(), 1, GL_FALSE, glm::value_ptr(view));

    glBindVertexArray(rayVAO);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);

    glUseProgram(0);
}



