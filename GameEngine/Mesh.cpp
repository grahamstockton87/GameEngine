#include "Mesh.h"
#include <iostream>


Mesh::Mesh() {
	VAO = 0;
	VBO = 0;
	IBO = 0;
	indexCount = 0;

	model = glm::mat4(1.0f);
}

Mesh::Mesh(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices)
{
	if (!vertices || !indices || numOfVertices == 0 || numOfIndices == 0) {
		std::cerr << "[Mesh::Mesh] ERROR: Invalid vertex/index data passed to constructor.\n";
		mVertices = nullptr;
		mIndices = nullptr;
		mNumOfVertices = 0;
		mNumOfIndices = 0;
		return;
	}

	VAO = 0;
	VBO = 0;
	IBO = 0;
	indexCount = 0;

	mNumOfVertices = numOfVertices;
	mNumOfIndices = numOfIndices;

	// Allocate and copy vertex data
	mVertices = new GLfloat[mNumOfVertices];
	std::memcpy(mVertices, vertices, sizeof(GLfloat) * mNumOfVertices);

	// Allocate and copy index data
	mIndices = new unsigned int[mNumOfIndices];
	std::memcpy(mIndices, indices, sizeof(unsigned int) * mNumOfIndices);

	model = glm::mat4(1.0f);
}
Mesh::Mesh(const Mesh& other)
{
	mNumOfVertices = other.mNumOfVertices;
	mNumOfIndices = other.mNumOfIndices;

	mVertices = new GLfloat[mNumOfVertices];
	std::memcpy(mVertices, other.mVertices, sizeof(GLfloat) * mNumOfVertices);

	mIndices = new unsigned int[mNumOfIndices];
	std::memcpy(mIndices, other.mIndices, sizeof(unsigned int) * mNumOfIndices);

	model = other.model;
}


void Mesh::CreateMesh()
{
	indexCount = mNumOfIndices;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(mIndices[0]) * mNumOfIndices, mIndices, GL_STATIC_DRAW);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mVertices[0])*mNumOfVertices, mVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mVertices[0]) * 8, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(mVertices[0]) * 8, (void*)(sizeof(mVertices[0]) * 3));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(mVertices[0]) * 8, (void*)(sizeof(mVertices[0]) * 5));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void Mesh::RenderMesh()
{
	if (IsValid) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

void Mesh::ClearMesh()
{
	if (IBO != 0) {
		glDeleteBuffers(1, &IBO);
		IBO = 0;
	}
	if (VBO != 0) {
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}
	if (VAO != 0) {
		glDeleteVertexArrays(1, &VAO);
		VAO = 0;
	}

	indexCount = 0;

	mVertices = nullptr;
	mIndices = nullptr;
	mNumOfIndices = 0;
	mNumOfVertices = 0;
	IsValid = false;
	//box.clear();
}


void Mesh::CalculateModelSpaceBoundingBox() {
	const unsigned int floatsPerVertex = 8;
	if (!mVertices || mNumOfVertices == 0 || mNumOfVertices % floatsPerVertex != 0) return;

	unsigned int vertexCount = mNumOfVertices / floatsPerVertex;

	GLfloat minX = mVertices[0];
	GLfloat minY = mVertices[1];
	GLfloat minZ = mVertices[2];
	GLfloat maxX = minX;
	GLfloat maxY = minY;
	GLfloat maxZ = minZ;

	for (unsigned int i = 1; i < vertexCount; ++i) {
		unsigned int offset = i * floatsPerVertex;
		GLfloat x = mVertices[offset];
		GLfloat y = mVertices[offset + 1];
		GLfloat z = mVertices[offset + 2];

		if (x < minX) minX = x;
		if (y < minY) minY = y;
		if (z < minZ) minZ = z;

		if (x > maxX) maxX = x;
		if (y > maxY) maxY = y;
		if (z > maxZ) maxZ = z;
	}

	untransformedBox.min = glm::vec3(minX, minY, minZ);
	untransformedBox.max = glm::vec3(maxX, maxY, maxZ);
}


void Mesh::transformBoundingBox()  {

	glm::vec3 min = untransformedBox.min;
	glm::vec3 max = untransformedBox.max;

	glm::vec3 corners[8] = {
		{ min.x, min.y, min.z },
		{ max.x, min.y, min.z },
		{ min.x, max.y, min.z },
		{ max.x, max.y, min.z },
		{ min.x, min.y, max.z },
		{ max.x, min.y, max.z },
		{ min.x, max.y, max.z },
		{ max.x, max.y, max.z }
	};


	glm::vec3 transformedMin = glm::vec3(model * glm::vec4(corners[0], 1.0f));
	glm::vec3 transformedMax = transformedMin;

	for (int i = 1; i < 8; ++i) {
		glm::vec3 transformed = glm::vec3(model * glm::vec4(corners[i], 1.0f));
		transformedMin = glm::min(transformedMin, transformed);
		transformedMax = glm::max(transformedMax, transformed);
	}

	box = BoundingBox{ transformedMin, transformedMax };
}




std::ostream& operator<<(std::ostream& os, const Mesh& mesh) {
	os << "Mesh Info:\n";
	os << " - Vertex count (floats): " << mesh.mNumOfVertices << "\n";
	os << " - Index count: " << mesh.mNumOfIndices << "\n";

	unsigned int vertexCount = mesh.mNumOfVertices / 8;
	unsigned int maxVerticesToPrint = 10;

	for (unsigned int i = 0; i < std::min(vertexCount, maxVerticesToPrint); i++) {
		GLfloat x = mesh.mVertices[i * 8 + 0];
		GLfloat y = mesh.mVertices[i * 8 + 1];
		GLfloat z = mesh.mVertices[i * 8 + 2];

		os << "Vertex " << i << ": ("
			<< x << ", " << y << ", " << z << ") UVS " << mesh.mVertices[i * 8 + 3] << mesh.mVertices[i * 8 + 4] << mesh.mVertices[i * 8 + 5] << mesh.mVertices[i * 8 + 6] << mesh.mVertices[i * 8 + 7] << std::endl;
	}

	return os;
}



void Mesh::translate(GLfloat x, GLfloat y, GLfloat z)
{
	model = glm::translate(model, glm::vec3(x, y, z));
}

void Mesh::rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	model = glm::rotate(model, angle * toRadians, glm::vec3(x, y, z));
}

void Mesh::scale(GLfloat x, GLfloat y, GLfloat z)
{
	model = glm::scale(model, glm::vec3(x, y, z));
}

void Mesh::translateBoundingBox(float x, float y, float z)
{
	box.min.x += x; box.min.y += y; box.min.z += z;
	box.max.x += x; box.max.y += y; box.max.z += z;
}

void Mesh::ModelReset()
{
	model = glm::mat4(1.0f);
}


Mesh::~Mesh()
{
	ClearMesh();
}
