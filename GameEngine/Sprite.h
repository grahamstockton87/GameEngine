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

class Sprite {
public:
	Sprite();
	Sprite(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices);

	void CreateSprite();
	void RenderSprite();
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
};

