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

	transformedVertices.clear();
	IsValid = false;
}


BoundingBox Mesh::CalculateBoundingBox() const
{
	BoundingBox box;

	if (transformedVertices.empty()) {
		box.min = glm::vec3(0.0f);
		box.max = glm::vec3(0.0f);
		//std::cerr << "[Warning] Bounding box calculation: No transformed vertices.\n";
		return box;
	}
	if (box.min.y == box.max.y) {
		box.min.y -= 0.1f;
		box.max.y += 0.1f;
	}


	auto minBound = transformedVertices[0];
	auto maxBound = transformedVertices[0];

	for (const auto& vertex : transformedVertices) {
		minBound = glm::min(minBound, vertex);
		maxBound = glm::max(maxBound, vertex);
	}

	box.min = minBound;
	box.max = maxBound;

	// Uncomment for debugging
	 //std::cout << "Bounding Box Min: " << box.min.x << ", " << box.min.y << ", " << box.min.z << '\n';
	 //std::cout << "Bounding Box Max: " << box.max.x << ", " << box.max.y << ", " << box.max.z << '\n';

	return box;
}


void Mesh::updateVertices()
{
	transformedVertices.clear();
	const unsigned int floatsPerVertex = 8;

	if (!mVertices || mNumOfVertices == 0 || mNumOfVertices % floatsPerVertex != 0) {
		//std::cerr << "[updateVertices] ERROR: Vertex data is invalid.\n";
		return;
	}

	unsigned int vertexCount = mNumOfVertices / floatsPerVertex;
	transformedVertices.resize(vertexCount); // avoid realloc every frame

	for (unsigned int i = 0; i < vertexCount; ++i) {
		unsigned int offset = i * floatsPerVertex;

		if (offset + 2 >= mNumOfVertices) {
			//std::cerr << "[updateVertices] ERROR: Offset out of range\n";
			break;
		}

		GLfloat x = mVertices[offset];
		GLfloat y = mVertices[offset + 1];
		GLfloat z = mVertices[offset + 2];

		transformedVertices[i] = glm::vec3(model * glm::vec4(x, y, z, 1.0f));
	}
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

void Mesh::ModelReset()
{
	model = glm::mat4(1.0f);
}


Mesh::~Mesh()
{
	ClearMesh();
}
