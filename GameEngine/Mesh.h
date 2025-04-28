#pragma once
#include <GL/glew.h>
#include <glm.hpp>
#include <xpolymorphic_allocator.h>
#include <vector>

class Mesh {

public:
	Mesh();
	Mesh(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices);
	void CreateMesh();
	void RenderMesh();
	void ClearMesh();

	GLfloat* mVertices;
	unsigned int* mIndices;
	unsigned int mNumOfVertices;
	unsigned int mNumOfIndices;

	struct BoundingBox {
		glm::vec3 min;
		glm::vec3 max;
	};

	BoundingBox CalculateBoundingBox() const;
	void updateVertices(glm::mat4 model);

	BoundingBox box;
	std::vector<glm::vec3> transformedVertices;

	~Mesh();
private:
	GLuint VAO, VBO, IBO;
	GLsizei indexCount;

};

