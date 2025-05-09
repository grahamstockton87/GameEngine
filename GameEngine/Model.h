#pragma once
#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>

#include "Mesh.h"
#include "Texture.h"

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
	void RenderModel();
	void ClearModel();
	void updateVerticesMesh();

	std::vector<std::unique_ptr<Mesh>>&& GetMeshList() { return std::move(meshList); }


	~Model();
private:
	void LoadNode(aiNode* node, const aiScene* scene);
	void LoadMesh(aiMesh* mesh, const aiScene* scene);
	void LoadMaterials(const aiScene* scene);

	std::vector<std::unique_ptr<Mesh>> meshList;
	std::vector<std::unique_ptr<Texture>> textureList;
	std::vector<unsigned int> meshToTex;
};

