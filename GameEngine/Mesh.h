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
#include "Triangle.h"


#include "BoundingBox.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;

	int boneIDs[4] = { 0 };
	float weights[4] = { 0.0f };

	void AddBoneData(int boneID, float weight) {
		for (int i = 0; i < 4; ++i) {
			if (weights[i] == 0.0f) {
				boneIDs[i] = boneID;
				weights[i] = weight;
				return;
			}
		}
		// Warn if more than 4 bones per vertex
	}
};

class Mesh {

public:
	Mesh();
	Mesh(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices);
	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
	Mesh(const Mesh& other);

	void CreateMesh();
	void RenderMesh();
	void ClearMesh();

	GLfloat* mVertices = nullptr;
	unsigned int* mIndices = nullptr;
	unsigned int mNumOfVertices = 0;
	unsigned int mNumOfIndices = 0;

	std::vector<Triangle> triangleList;
	void UpdateTriangleList() { triangleList.clear(); triangleList = ExtractTrianglesFromMesh(); }

	void CalculateModelSpaceBoundingBox();

	BoundingBox box, untransformedBox;

	BoundingBox getBoundingBox() { return box; }
	void transformBoundingBox();

	friend std::ostream& operator<<(std::ostream& os, const Mesh& mesh);

	bool UsesBoxCollision = true;

	void UpdateBuffers();

	void translate(GLfloat x, GLfloat y, GLfloat z);
	void rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
	void scale(GLfloat x, GLfloat y, GLfloat z);

	void translateBoundingBox(float x, float y, float z);

	glm::mat4 GetModel() { return model; }
	void SetModel(glm::mat4 modelIn) { model = modelIn; }

	glm::mat4 model;

	std::string ID = "null";

	void ResetModel();

	bool IsValid = true;

	bool shootable = false;

	bool moveToTopOfBox = true;

	bool rigid = true;

	std::vector<Vertex> vertexData;
	std::vector<unsigned int> indexData;

	bool hasAnimation = false;

	~Mesh();
private:
	GLuint VAO, VBO, IBO;

	GLsizei indexCount;

	const float toRadians = 3.14159265f / 180.0f;

	std::vector<Triangle> ExtractTrianglesFromMesh();


};

