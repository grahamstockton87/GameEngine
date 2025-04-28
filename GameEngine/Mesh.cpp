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
		return box;
	}

	// Initialize min and max with the first transformed vertex
	box.min = transformedVertices[0];
	box.max = transformedVertices[0];

	// Loop through all transformed vertices to update the bounds
	for (const auto& vertex : transformedVertices) {
		box.min = glm::min(box.min, vertex);
		box.max = glm::max(box.max, vertex);
	}
	//std::cout << box.min.x << box.min.y << box.min.z << box.max.x;
	return box;
}


void Mesh::updateVertices(glm::mat4 model)
{
	transformedVertices.clear();

	for (unsigned int i = 0; i < mNumOfVertices/ 8 * 4; i+=4) {

		// Read position
		GLfloat x = mVertices[i + 0];
		GLfloat y = mVertices[i + 1];
		GLfloat z = mVertices[i + 2];

		glm::vec4 localPos = glm::vec4(x, y, z, 1.0f);
		glm::vec4 worldPos = model * localPos;

		//std::cout << worldPos[0] << " " << worldPos[1] << " " << worldPos[2] << std::endl;

		transformedVertices.push_back(glm::vec3(worldPos.x,worldPos.y,worldPos.z));
	}
}


Mesh::~Mesh()
{
	ClearMesh();
}
