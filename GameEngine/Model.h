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
#include <unordered_map>

#include "Animation.h"
#include "Shader.h"

class Model {
public:
	Model();

	// copy
		// deep-copy
	Model(const Model& other)
		: box(other.box),
		untransformedBox(other.untransformedBox),
		shootable(other.shootable),
		IsValid(other.IsValid),
		rigid(other.rigid),
		UsesBoxCollision(other.UsesBoxCollision),
		model(other.model)
	{
		// 1) Deep-copy meshes
		meshList.clear();

		for (auto const& mptr : other.meshList) {
			// Assuming Mesh has a copy ctor:
			meshList.push_back(std::make_unique<Mesh>(*mptr));
		}

		// 2) Deep-copy textures (if textureList holds unique_ptr<Texture>)
		textureList.clear();
		textureList.reserve(other.textureList.size());
		for (auto const& tptr : other.textureList) {
			textureList.push_back(std::make_unique<Texture>(*tptr));
		}
	}
	Model& operator=(const Model&) = delete;

	// Allow move
	Model(Model&&) noexcept = default;
	Model& operator=(Model&&) noexcept = default;

	void LoadModel(const std::string fileName);
	void LoadMeshBones(aiMesh* mesh, std::vector<Vertex>& vertices);
	void RenderModel();
	void RenderModel(Texture* tex);
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
	std::vector<std::unique_ptr<Texture>> specularMapList;
	std::vector<unsigned int> meshToTex;

	struct BoneInfo {
		glm::mat4 offsetMatrix;  // from aiBone
		glm::mat4 finalTransformation; // for shader
	};
	std::vector<BoneInfo> boneInfo;
	std::unique_ptr<Animation> animation;
	std::unordered_map<std::string, int> boneMapping;
	int boneCount = 0;
	bool hasAnimation = false;
	std::unique_ptr<Animator> animator;

};

