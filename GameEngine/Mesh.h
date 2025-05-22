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

	void CalculateModelSpaceBoundingBox();

	BoundingBox box, untransformedBox;

	BoundingBox getBoundingBox() { return box; }
	void transformBoundingBox();

	friend std::ostream& operator<<(std::ostream& os, const Mesh& mesh);

	bool UsesBoxCollision = true;

	void translate(GLfloat x, GLfloat y, GLfloat z);
	void rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
	void scale(GLfloat x, GLfloat y, GLfloat z);

	void translateBoundingBox(float x, float y, float z);

	glm::mat4 GetModel() { return model; }
	void SetModel(glm::mat4 modelIn) { model = modelIn; }
	glm::mat4 model;

	std::string ID = "null";

	void ModelReset();
	bool IsValid = true;

	bool shootable = false;

	bool moveToTopOfBox = true;
	~Mesh();
private:
	GLuint VAO, VBO, IBO;

	GLsizei indexCount;

	const float toRadians = 3.14159265f / 180.0f;
};

