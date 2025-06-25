#include "Model.h"

Model::Model(){}

void Model::LoadModel(const std::string fileName)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(fileName,
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		aiProcess_LimitBoneWeights |
		aiProcess_JoinIdenticalVertices);

	if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
		printf("ERROR: Failed to load model %s: %s\n", fileName.c_str(), importer.GetErrorString());
		return;
	}

	if (scene->HasAnimations()) {
		hasAnimation = true;

		// Load animation into our custom class
		animation = std::make_unique<Animation>(scene->mAnimations[0], scene);

		// Integrate boneInfo from the animation into model
		for (const auto& bone : animation->bones) {
			const std::string& boneName = bone.name;
			int boneID = bone.id;

			if (boneMapping.find(boneName) == boneMapping.end()) {
				boneMapping[boneName] = boneID;

				if ((int)boneInfo.size() <= boneID)
					boneInfo.resize(boneID + 1);

				boneInfo[boneID].offsetMatrix = glm::mat4(1.0f); // You can set real offset in LoadMeshBones
			}
		}
	}

	LoadNode(scene->mRootNode, scene);
	LoadMaterials(scene);

	for (auto& mesh : meshList) {
		mesh->rigid = rigid;
		mesh->hasAnimation = hasAnimation;
	}

	if (scene->HasAnimations()) {
		hasAnimation = true;
		animation = std::make_unique<Animation>(scene->mAnimations[0], scene);
		animator = std::make_unique<Animator>(animation.get());
	}


}
void Model::LoadMaterials(const aiScene* scene)
{
	textureList.resize(scene->mNumMaterials);
	specularMapList.resize(scene->mNumMaterials);

	for (size_t i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial* material = scene->mMaterials[i];
		textureList[i] = nullptr;

		if (material->GetTextureCount(aiTextureType_DIFFUSE)) {
			aiString path;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
				std::string texPath = "Textures/" + std::string(path.C_Str());
				auto texture = std::make_unique<Texture>(texPath);
				if (!texture->LoadTexture()) {
					texture->LoadTextureA();
				}
				textureList[i] = std::move(texture);
			}
		}
		if (!textureList[i]) {
			auto defaultTex = std::make_unique<Texture>("Textures/plain.png");
			defaultTex->LoadTextureA();
			textureList[i] = std::move(defaultTex);
		}
		if (material->GetTextureCount(aiTextureType_SPECULAR)) {
			aiString path;
			if (material->GetTexture(aiTextureType_SPECULAR, 0, &path) == AI_SUCCESS) {
				std::string texPath = "Textures/" + std::string(path.C_Str());
				auto texture = std::make_unique<Texture>(texPath);
				if (!texture->LoadTexture()) {
					texture->LoadTextureA();
				}
				specularMapList[i] = std::move(texture);
			}
		}
		if (!specularMapList[i]) {
			auto defaultTex = std::make_unique<Texture>("Textures/plain.png");
			defaultTex->LoadTextureA();
			specularMapList[i] = std::move(defaultTex);
		}
		// Create Material List
		auto mat = std::make_unique<Material>();
		mat->SetSpecularMap(specularMapList[i].get());
	}
}

void Model::LoadMeshBones(aiMesh* mesh, std::vector<Vertex>& vertices)
{
	for (unsigned int i = 0; i < mesh->mNumBones; ++i) {
		aiBone* aiBone = mesh->mBones[i];
		std::string boneName(aiBone->mName.C_Str());

		// If this bone isn't in our mapping yet, assign it an ID
		int boneID = 0;
		if (boneMapping.find(boneName) == boneMapping.end()) {
			boneID = boneCount++;
			boneMapping[boneName] = boneID;

			if (boneID >= boneInfo.size())
				boneInfo.resize(boneID + 1);

			// Convert Assimp offset matrix to glm
			boneInfo[boneID].offsetMatrix = glm::transpose(glm::make_mat4(&aiBone->mOffsetMatrix.a1));
		}
		else {
			boneID = boneMapping[boneName];
		}

		// Loop through bone weights and assign them to the vertex
		for (unsigned int j = 0; j < aiBone->mNumWeights; ++j) {
			unsigned int vertexID = aiBone->mWeights[j].mVertexId;
			float weight = aiBone->mWeights[j].mWeight;

			if (vertexID < vertices.size()) {
				vertices[vertexID].AddBoneData(boneID, weight);
			}
		}
	}
}



void Model::RenderModel()
{
	if (meshList.empty()) return;
	for (size_t i = 0; i < meshList.size(); i++) {
		unsigned int materialIndex = meshToTex[i];
		if (materialIndex < textureList.size() && textureList[materialIndex]) {
			textureList[materialIndex]->UseTexture();
		}
		meshList[i]->RenderMesh();
	}
}
void Model::RenderModel(Texture* tex)
{
	tex->UseTexture();
	for (size_t i = 0; i < meshList.size(); i++) {
		meshList[i]->RenderMesh();
	}
}



void Model::ClearModel()
{
	meshList.clear();       // unique_ptr auto-deletes
	textureList.clear();    // unique_ptr auto-deletes
	for (const auto& mesh : meshList) {
		mesh->ClearMesh();
	}
}

void Model::ResetModel()
{
	model = glm::mat4(1.0f);
}

Model::~Model()
{
	ClearModel();
}

void Model::LoadNode(aiNode* node, const aiScene* scene)
{
	for (size_t i = 0; i < node->mNumMeshes; i++) {
		LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
	}
	for (size_t i = 0; i < node->mNumChildren; i++) {
		LoadNode(node->mChildren[i], scene);
	}
}

void Model::LoadMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;

	for (size_t i = 0; i < mesh->mNumVertices; i++) {
		vertices.insert(vertices.end(), { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });
		if (mesh->mTextureCoords[0]) {
			vertices.insert(vertices.end(), { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
		}
		else {
			vertices.insert(vertices.end(), { 0.0f, 0.0f });
		}
		vertices.insert(vertices.end(), { -mesh->mNormals[i].x, -mesh->mNormals[i].y, -mesh->mNormals[i].z });
	}

	for (size_t i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	auto newMesh = std::make_unique<Mesh>(&vertices[0], &indices[0], vertices.size(), indices.size());
	newMesh->CreateMesh();
	meshList.push_back(std::move(newMesh));
	meshToTex.push_back(mesh->mMaterialIndex);
}





void Model::translate(GLfloat x, GLfloat y, GLfloat z)
{
	model = glm::translate(model, glm::vec3(x, y, z));
	for (auto& mesh : meshList)
		mesh->model = model;
}

void Model::rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	model = glm::rotate(model, glm::radians(angle), glm::vec3(x, y, z));
	for (auto& mesh : meshList)
		mesh->model = model;
}

void Model::scale(GLfloat x, GLfloat y, GLfloat z)
{
	model = glm::scale(model, glm::vec3(x, y, z));
	for (auto& mesh : meshList)
		mesh->model = model;
	//std::cout << " MODEL: " << glm::to_string(model) << std::endl;
}
void Model::scaleUVs(float scale) {
	for (auto& mesh : meshList) {
		if (!mesh) continue;

		int floatsPerVertex = 8;

		int numVertices = mesh->mNumOfVertices / floatsPerVertex;

		for (int i = 0; i < numVertices; ++i) {
			int offset = i * floatsPerVertex;

			mesh->mVertices[offset + 3] *= scale; // U
			mesh->mVertices[offset + 4] *= scale; // V
		}
		mesh->CreateMesh(); 
	}
}

void Model::CalculateModelSpaceBoundingBox() {
	if (meshList.empty()) return;

	const unsigned int floatsPerVertex = 8;
	glm::vec3 overallMin(FLT_MAX);
	glm::vec3 overallMax(-FLT_MAX);
	bool sawAnyVertex = false;

	for (const auto& mesh : meshList) {
		const GLfloat* v = mesh->mVertices;
		unsigned int numFloats = mesh->mNumOfVertices;
		if (!v || numFloats < floatsPerVertex) continue;

		unsigned int vertexCount = numFloats / floatsPerVertex;
		sawAnyVertex = sawAnyVertex || (vertexCount > 0);

		// pointer‐based loop: very little overhead per iteration
		const float* p = v;
		for (unsigned int i = 0; i < vertexCount; ++i, p += floatsPerVertex) {
			// only read the XYZ components
			glm::vec3 pos(p[0], p[1], p[2]);

			// branchless min/max
			overallMin = glm::min(overallMin, pos);
			overallMax = glm::max(overallMax, pos);
		}
	}

	if (!sawAnyVertex) return;  // nothing valid to bound

	// store the untransformed box
	untransformedBox = BoundingBox{ overallMin, overallMax };

	// —— Now compute the world‐space AABB by transforming the center & extents ——
	// 1) compute model‐space center & half‐extents
	glm::vec3 center = (overallMin + overallMax) * 0.5f;
	glm::vec3 extents = (overallMax - overallMin) * 0.5f;

	// 2) transform the center (one 4×4 multiply)
	glm::vec3 worldCenter = glm::vec3(model * glm::vec4(center, 1.0f));

	// 3) extract the linear (3×3) part of the model matrix
	glm::mat3 linearMat(model);

	// 4) form the absolute‐value matrix
	glm::mat3 absMat(
		std::abs(linearMat[0][0]), std::abs(linearMat[0][1]), std::abs(linearMat[0][2]),
		std::abs(linearMat[1][0]), std::abs(linearMat[1][1]), std::abs(linearMat[1][2]),
		std::abs(linearMat[2][0]), std::abs(linearMat[2][1]), std::abs(linearMat[2][2])
	);

	// 5) compute the world‐space half‐extents
	glm::vec3 worldExtents = absMat * extents;

	// 6) assemble the final AABB
	box.min = worldCenter - worldExtents;
	box.max = worldCenter + worldExtents;
}


void Model::CheckBoxCollisionModel(bool& hit, bool& hitSide, float& closestY, float groundY, glm::vec3 delta, const bool moveToTopOfBox, Camera& camera, GLfloat deltaTime)
{
	for (auto& mesh : meshList) {
		CheckBoxCollision(hit, hitSide, closestY, groundY, delta, mesh->getBoundingBox(), moveToTopOfBox, camera, deltaTime);
	}
}

void Model::CheckTriangleCollisionModel(float& closestY, bool& hit, bool& hitSide, bool& hitTop, glm::vec3& wallNormal, Camera& camera)
{
	for (auto& mesh : meshList) {
		CheckTriangleCollision(closestY, hit, hitSide, hitTop, wallNormal, mesh, camera);
	}
}


void Model::transformBoundingBox() {

	glm::vec3 min = untransformedBox.min;
	glm::vec3 max = untransformedBox.max;

	glm::vec3 corners[8] = {
		{ min.x, min.y, min.z },
		{ max.x, min.y, min.z },
		{ min.x, max.y, min.z },
		{ max.x, max.y, min.z },
		{ min.x, min.y, max.z },
		{ max.x, min.y, max.z },
		{ min.x, max.y, max.z },
		{ max.x, max.y, max.z }
	};


	glm::vec3 transformedMin = glm::vec3(meshList[0]->GetModel() * glm::vec4(corners[0], 1.0f));
	glm::vec3 transformedMax = transformedMin;

	for (int i = 1; i < 8; ++i) {
		glm::vec3 transformed = glm::vec3(meshList[0]->GetModel() * glm::vec4(corners[i], 1.0f));
		transformedMin = glm::min(transformedMin, transformed);
		transformedMax = glm::max(transformedMax, transformed);
	}

	box = BoundingBox{ transformedMin, transformedMax };
}

void Model::SetRigid(bool rigidP)
{
	for (auto& mesh : meshList) {
		mesh->rigid = rigidP;
	}
	rigid = rigidP;
}
void Model::UpdateTriangleList() {
	for (auto& mesh : meshList) {
		mesh->UpdateTriangleList();
	}
}

