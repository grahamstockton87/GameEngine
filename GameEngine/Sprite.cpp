#include "Sprite.h"

void safeDelete(GLfloat*& ptr) {
	if (ptr) {
		delete[] ptr;
		ptr = nullptr;
	}
}
void safeDelete(unsigned int*& ptr) {
	if (ptr) {
		delete[] ptr;
		ptr = nullptr;
	}
}

Sprite::Sprite(const Sprite& other)
	: mNumVertices(other.mNumVertices)
	, mNumIndices(other.mNumIndices)
	, indexCount(other.indexCount)
	, texture(other.texture)
{
	// 1) Copy CPU data
	mVertices = new GLfloat[mNumVertices];
	std::memcpy(mVertices, other.mVertices, sizeof(GLfloat) * mNumVertices);

	mIndices = new unsigned int[mNumIndices];
	std::memcpy(mIndices, other.mIndices, sizeof(unsigned int) * mNumIndices);

	// 2) Create & upload new GPU buffers
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(unsigned int) * mNumIndices,
		mIndices,
		GL_STATIC_DRAW);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(GLfloat) * mNumVertices,
		mVertices,
		GL_STATIC_DRAW);

	// 3) Re-establish the vertex attributes exactly as in CreateSprite()
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
		(void*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// 4) Unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Sprite& Sprite::operator=(const Sprite& other) {
	if (this == &other) return *this;

	// 1) Clean up existing resources
	ClearMesh();  // assumes this deletes VAO/VBO/EBO and CPU arrays

	// 2) Copy sizes & counts
	mNumVertices = other.mNumVertices;
	mNumIndices = other.mNumIndices;
	indexCount = other.indexCount;
	texture = other.texture;

	// 3) Copy CPU data
	mVertices = new GLfloat[mNumVertices];
	std::memcpy(mVertices, other.mVertices, sizeof(GLfloat) * mNumVertices);

	mIndices = new unsigned int[mNumIndices];
	std::memcpy(mIndices, other.mIndices, sizeof(unsigned int) * mNumIndices);

	// 4) Re-create GPU buffers exactly as above
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(unsigned int) * mNumIndices,
		mIndices,
		GL_STATIC_DRAW);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(GLfloat) * mNumVertices,
		mVertices,
		GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
		(void*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return *this;

}
Sprite::Sprite()
	: VAO(0)
	, VBO(0)
	, EBO(0)
	, indexCount(0)
	, mVertices(nullptr)
	, mIndices(nullptr)
	, mNumVertices(0)
	, mNumIndices(0)
	, texture(nullptr)
{
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
// In Sprite.cpp
void Sprite::SetFrameUVs(int frameIndex, int totalFrames) {
	// clamp params
	if (totalFrames <= 0) return;
	frameIndex = glm::clamp(frameIndex, 0, totalFrames - 1);

	// how wide each frame is in UV space:
	float frameW = 1.0f / float(totalFrames);
	float u0 = frameIndex * frameW;
	float u1 = u0 + frameW;

	// we leave v from 0→1 to span full texture height:
	// vertex order: BL, BR, TR, TL
	mVertices[2] = u0;  // BL.u
	mVertices[6] = u1;  // BR.u
	mVertices[10] = u1;  // TR.u
	mVertices[14] = u0;  // TL.u
	// (v coords stay mVertices[3], [7], [11], [15])

	// re-upload UVs (or entire buffer) to GPU:
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		sizeof(GLfloat) * mNumVertices,
		mVertices
	);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void Sprite::UpdateVertices(GLfloat* vertices, unsigned int* indices)
{

	// Allocate and copy vertex data
	mVertices = new GLfloat[mNumVertices];
	std::memcpy(mVertices, vertices, sizeof(GLfloat) * mNumVertices);

	// Allocate and copy index data
	mIndices = new unsigned int[mNumIndices];
	std::memcpy(mIndices, indices, sizeof(unsigned int) * mNumIndices);

	// now push to GPU:
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mNumVertices, mVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mNumIndices, mIndices, GL_STATIC_DRAW);
	// unbind if you like…
}


void Sprite::Render(Shader& shader, const glm::mat4& orthoProj) {
	if (!texture) return;

	if (VAO == 0 || VBO == 0 || EBO == 0) {
		std::cerr << "[Sprite::Render] ERROR: Sprite not initialized properly.\n";
		return;
	}

	// 1) Prepare GL state
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shader.UseShader();
	glUniformMatrix4fv(shader.GetProjectionLocation(),1, GL_FALSE,glm::value_ptr(orthoProj));
	

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	
	glActiveTexture(GL_TEXTURE0);
	texture->UseTexture(GL_TEXTURE0);
	glUniform1i(shader.GetUniformSpriteTextureLocation(), 0);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	// 5) Unbind VAO and texture
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// restore GL state
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
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
