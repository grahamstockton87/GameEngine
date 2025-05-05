#include "Mesh.h"
#include <iostream>


Mesh::Mesh() {
	VAO = 0;
	VBO = 0;
	IBO = 0;
	indexCount = 0;
}

Mesh::Mesh(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices)
{
	mVertices = vertices;
	mIndices = indices;
	mNumOfVertices = numOfVertices;
	mNumOfIndices = numOfIndices;
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
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
}

Mesh::BoundingBox Mesh::CalculateBoundingBox() const
{
	BoundingBox box;

	if (transformedVertices.empty()) {
		box.min = glm::vec3(0.0f);
		box.max = glm::vec3(0.0f);
		std::cerr << "[Warning] Bounding box calculation: No transformed vertices.\n";
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


void Mesh::updateVertices(glm::mat4 model)
{
	transformedVertices.clear();

	unsigned int vertexCount = mNumOfVertices / 8;
	for (unsigned int i = 0; i < vertexCount; i++) {
		GLfloat x = mVertices[i * 8 + 0];
		GLfloat y = mVertices[i * 8 + 1];
		GLfloat z = mVertices[i * 8 + 2];

		glm::vec4 localPos = glm::vec4(x, y, z, 1.0f);
		glm::vec4 worldPos = model * localPos;

		transformedVertices.push_back(glm::vec3(worldPos));
		//std::cout << "Vertex " << i << ": " << x << ", " << y << ", " << z << "\n";
		//std::cout << worldPos.x << worldPos.y << worldPos.z;
	}
}



Mesh::~Mesh()
{
	ClearMesh();
}
