#include "RenderUtils.h"
#include <iostream>

namespace RenderUtils {


    static GLuint quadVAO = 0, quadVBO = 0;

    /// Draws a fullscreen quad with up to two textures.
    /// shader      – the Shader you want to use
    /// tex0, uni0  – first texture + its sampler name (e.g. "currentFrame")
    /// tex1, uni1  – (optional) second texture + its sampler name (e.g. "previousFrame")
    /// blend       – (optional) value for a "blendFactor" uniform
    void DrawFullScreenQuad(Shader& shader,
        GLuint tex0, const char* uni0,
        GLuint tex1, const char* uni1)
    {
        // 1) Lazy-create the quad VAO/VBO
        if (quadVAO == 0) {
            float quadVerts[] = {
                // pos      // tex
                -1,  1,      0, 1,
                -1, -1,      0, 0,
                 1, -1,      1, 0,

                -1,  1,      0, 1,
                 1, -1,      1, 0,
                 1,  1,      1, 1,
            };
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

            // aPos
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            // aTexCoords
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

            glBindVertexArray(0);
        }

        // 2) Bind shader + textures + uniforms
        shader.UseShader();
        // texture 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex0);
        GLint loc0 = glGetUniformLocation(shader.GetShaderID(), uni0);
        if (loc0 >= 0) glUniform1i(loc0, 0);

        // texture 1 (if provided)
        if (uni1) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, tex1);
            GLint loc1 = glGetUniformLocation(shader.GetShaderID(), uni1);
            if (loc1 >= 0) glUniform1i(loc1, 1);
        }

        // 3) Draw
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
    void DrawFullScreenQuad(Shader& shader, GLuint tex) {
        // 1) Lazy-create the quad VAO/VBO
        if (quadVAO == 0) {
            float quadVerts[] = {
                // pos      // tex
                -1,  1,      0, 1,
                -1, -1,      0, 0,
                 1, -1,      1, 0,

                -1,  1,      0, 1,
                 1, -1,      1, 0,
                 1,  1,      1, 1,
            };
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

            // aPos
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            // aTexCoords
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

            glBindVertexArray(0);
        }

        // 2) Bind shader + textures + uniforms
        shader.UseShader();

        GLint loc = glGetUniformLocation(shader.GetShaderID(), "sceneTexture");
        if (loc >= 0) glUniform1i(loc, 0);

        // texture 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        // 3) Draw
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

    }
    void DrawFullScreenQuad(Shader& shader) {
        if (quadVAO == 0) {
            float quadVertices[] = {
                // positions   // texcoords
                -1.0f,  1.0f,  0.0f, 1.0f,
                -1.0f, -1.0f,  0.0f, 0.0f,
                 1.0f, -1.0f,  1.0f, 0.0f,

                -1.0f,  1.0f,  0.0f, 1.0f,
                 1.0f, -1.0f,  1.0f, 0.0f,
                 1.0f,  1.0f,  1.0f, 1.0f
            };
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        }

        shader.UseShader(); // cleaner and const-safe
        GLint loc = glGetUniformLocation(shader.GetShaderID(), "sceneTexture");
        if (loc >= 0) glUniform1i(loc, 0);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

}