#pragma once
#include <GL/glew.h>
#include <glm.hpp>
#include <xpolymorphic_allocator.h>

#include <gtc/matrix_transform.hpp>
#include <gtx/component_wise.hpp>   // For glm::all and glm::isfinite

#include <vector>
#include <string>
#include <cmath>
#include <iostream>

#include "Shader.h"
#include "Texture.h"

class Sprite {
public:
	Sprite();
	Sprite(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices);

	// Move semantics are now correct and defaulted:
	Sprite(Sprite&&)      noexcept = default;
	Sprite& operator=(Sprite&&) noexcept = default;

	// Non-copyable (because OpenGL VAOs aren’t trivially copyable)
	Sprite(const Sprite& other);
	Sprite& operator=(const Sprite& other);

	void CreateSprite();
	void UpdateVertices(GLfloat* vertices, unsigned int* indices);
	void SetFrameUVs(int frameIndex, int totalFrames);
	void Render(Shader& shader, const glm::mat4& orthoProj);
    void SetTexture(Texture* tex) { texture = tex; }
	void ClearMesh();
	void Scale(float x, float y);

	GLfloat* mVertices = nullptr;
	unsigned int* mIndices = nullptr;
	unsigned int mNumVertices;
	unsigned int mNumIndices;

	~Sprite();

private:
	GLuint VAO, VBO, EBO;
	GLsizei indexCount;
	Texture* texture;

};

