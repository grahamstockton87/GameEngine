#pragma once
#include <GL/glew.h>
#include <glm.hpp>
#include <xpolymorphic_allocator.h>

#include <gtc/matrix_transform.hpp>
#include <gtx/component_wise.hpp>   // For glm::all and glm::isfinite

#include <vector>
#include <string>
#include <cmath>

#include "BoundingBox.h"

class Mesh {

public:
	Mesh();
	Mesh(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices);
	Mesh(const Mesh& other);

	void CreateMesh();
	void RenderMesh();
	void ClearMesh();

	GLfloat* mVertices = nullptr;
	unsigned int* mIndices = nullptr;
	unsigned int mNumOfVertices = 0;
	unsigned int mNumOfIndices = 0;


	BoundingBox CalculateBoundingBox() const;
	void updateVertices();

	BoundingBox box;

	BoundingBox getBoundingBox() { return box; }

	friend std::ostream& operator<<(std::ostream& os, const Mesh& mesh);

	bool UsesBoxCollision = true;

	void translate(GLfloat x, GLfloat y, GLfloat z);
	void rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
	void scale(GLfloat x, GLfloat y, GLfloat z);

	glm::mat4 GetModel() { return model; }

	std::vector<glm::vec3> transformedVertices;

	std::string ID = "null";

	void ModelReset();
	bool IsValid = true;

	bool shootable = false;
	~Mesh();
private:
	GLuint VAO, VBO, IBO;

	GLsizei indexCount;


	glm::mat4 model;
	const float toRadians = 3.14159265f / 180.0f;
};

