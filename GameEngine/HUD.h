#pragma once

#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>

#include "Texture.h"
#include "Shader.h"

class HUD {
public:
    HUD() : VAO(0), VBO(0), EBO(0), width(0), height(0) {}

    void Initialize(float xCenter, float yTop, float spriteWidth, float spriteHeight, float xDisplacement = 0.0f) {
        width = spriteWidth;
        height = spriteHeight;

        GLfloat vertices[] = {
            // positions                                         // texCoords
            xDisplacement + xCenter - width / 2, yTop,           0.0f, 0.0f,
            xDisplacement + xCenter + width / 2, yTop,           1.0f, 0.0f,
            xDisplacement + xCenter + width / 2, yTop - height,  1.0f, 1.0f,
            xDisplacement + xCenter - width / 2, yTop - height,  0.0f, 1.0f
        };

        GLuint indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    void SetTexture(Texture* tex) {
        texture = tex;
    }

    void Render(Shader& shader, const glm::mat4& orthoProj) {
        if (!texture) return;

        shader.UseShader();
        glUniformMatrix4fv(
            shader.GetProjectionLocation(),
            1, GL_FALSE,
            glm::value_ptr(orthoProj)
        );

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        glActiveTexture(GL_TEXTURE0);
        texture->UseTexture(GL_TEXTURE0);
        glUniform1i(shader.GetUniformSpriteTextureLocation(), 0);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        // restore GL state
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }


    void Cleanup() {
        if (VBO) glDeleteBuffers(1, &VBO);
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }

    ~HUD() {
        Cleanup();
    }

private:
    GLuint VAO, VBO, EBO;
    float width, height;
    Texture* texture = nullptr;
};
