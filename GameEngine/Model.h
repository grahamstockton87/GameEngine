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


class Model {
public:
	Model();

	// Prevent copy
	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	// Allow move
	Model(Model&&) noexcept = default;
	Model& operator=(Model&&) noexcept = default;

	void LoadModel(const std::string fileName);
	void RenderModel(GLuint uniformModel);
	void RenderModel(GLuint uniformModelLocation, bool OverrideTexture);
	void ClearModel();
	void transformBoundingBox();

	void translate(GLfloat x, GLfloat y, GLfloat z);
	void rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
	void scale(GLfloat x, GLfloat y, GLfloat z);

	void CalculateModelSpaceBoundingBox();

	std::vector<std::unique_ptr<Mesh>>&& GetMeshList() { return std::move(meshList); }

	BoundingBox GetBox() { return box; }

	bool shootable = false;
	bool IsValid = true;

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

