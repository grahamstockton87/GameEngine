#pragma once
#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>

#include "Mesh.h"
#include "Texture.h"
#include <iostream>
#include <gtc/type_ptr.hpp>
#include <gtx/string_cast.hpp>
#include "Material.h"
#include "CollisionUtils.h"

class Model {
public:
	Model();

	// Prevent copy
	Model(const Model&) = default;
	Model& operator=(const Model&) = delete;

	// Allow move
	Model(Model&&) noexcept = default;
	Model& operator=(Model&&) noexcept = default;

	void LoadModel(const std::string fileName);
	void RenderModel(GLuint uniformModel);
	void RenderModel(GLuint uniformModelLocation, bool OverrideTexture);
	void ClearModel();
	void ResetModel();
	void transformBoundingBox();

	void SetRigid(bool rigidP);

	void translate(GLfloat x, GLfloat y, GLfloat z);
	void rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
	void scale(GLfloat x, GLfloat y, GLfloat z);
	void scaleUVs(float scale);

	void CalculateModelSpaceBoundingBox();

	void CheckBoxCollisionModel(bool& hit, bool& hitSide, float& closestY, float groundY, glm::vec3 delta, const bool moveToTopOfBox, Camera& camera, GLfloat deltaTime);
	void CheckTriangleCollisionModel(float& closestY, bool& hit, bool& hitSide, bool& hitTop, glm::vec3& wallNormal, Camera& camera);
	void UpdateTriangleList();


	std::vector<std::unique_ptr<Mesh>>& GetMeshList() { return meshList; }


	BoundingBox GetBox() { return box; }

	bool shootable = false;
	bool IsValid = true;
	bool rigid = true;
	bool UsesBoxCollision = true;

	glm::mat4 model = glm::mat4(1.0f);

	~Model();

private:
	void LoadNode(aiNode* node, const aiScene* scene);
	void LoadMesh(aiMesh* mesh, const aiScene* scene);
	void LoadMaterials(const aiScene* scene);

	BoundingBox box, untransformedBox;

	std::vector<std::unique_ptr<Mesh>> meshList;
	std::vector<std::unique_ptr<Texture>> textureList;
	std::vector<unsigned int> meshToTex;

};

