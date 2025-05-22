#include "Sprite.h"


Sprite::Sprite() {
	VAO = 0;
	VBO = 0;
	EBO = 0;
	indexCount = 0;

}

Sprite::Sprite(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices){
	if (!vertices || !indices || numOfVertices == 0 || numOfIndices == 0) {
		std::cerr << "[Mesh::Mesh] ERROR: Invalid vertex/index data passed to constructor.\n";
		mVertices = nullptr;
		mIndices = nullptr;
		mNumVertices = 0;
		mNumIndices = 0;
		return;
	}

	VAO = 0;
	VBO = 0;
	EBO = 0;
	indexCount = 0;

	mNumVertices = numOfVertices;
	mNumIndices = numOfIndices;
	indexCount = mNumIndices;  // Add this line to fix rendering


	// Allocate and copy vertex data
	mVertices = new GLfloat[mNumVertices];
	std::memcpy(mVertices, vertices, sizeof(GLfloat) * mNumVertices);

	// Allocate and copy index data
	mIndices = new unsigned int[mNumIndices];
	std::memcpy(mIndices, indices, sizeof(unsigned int) * mNumIndices);
}

void Sprite::CreateSprite() {
	indexCount = mNumIndices;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(mIndices[0]) * mNumIndices, mIndices, GL_STATIC_DRAW);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mVertices[0]) * mNumVertices, mVertices, GL_STATIC_DRAW);

	// Position (x, y)
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void*)0);
	glEnableVertexAttribArray(0);

	// TexCoords (u, v)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void*)(sizeof(GLfloat) * 2));
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);  // Leave EBO bound while VAO is active
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);  // Optional, outside VAO

}

void Sprite::RenderSprite()
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Sprite::ClearMesh()
{
	if(EBO != 0) {
		glDeleteBuffers(1, &EBO);
		EBO = 0;
	}
	if (VBO != 0) {
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}
	if (VAO != 0) {
		glDeleteVertexArrays(1, &VAO);
		VAO = 0;
	}


	mVertices = nullptr;
	mIndices = nullptr;
	mNumVertices = 0;
	mNumIndices = 0;
	indexCount = 0;
}

void Sprite::Scale(float x, float y)
{
	for (int i = 0; i < mNumVertices; i += 2) {
		mVertices[i] *= x;
		mVertices[i + 1] *= y;
	}
}

Sprite::~Sprite()
{
	ClearMesh();
}
