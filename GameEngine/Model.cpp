#include "Model.h"

Model::Model(){}

void Model::LoadModel(const std::string fileName)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(fileName,
		aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

	if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
		printf("ERROR: Failed to load model %s: %s\n", fileName.c_str(), importer.GetErrorString());
		return;
	}

	printf("SUCCESS: Loaded model %s\n", fileName.c_str());
	printf("Scene has %d meshes and %d materials\n", scene->mNumMeshes, scene->mNumMaterials);

	LoadNode(scene->mRootNode, scene);
	LoadMaterials(scene);

	printf("Model loading complete. Mesh count: %zu\n", meshList.size());
}

void Model::RenderModel(GLuint uniformModelLocation)
{
	for (size_t i = 0; i < meshList.size(); i++) {
		unsigned int materialIndex = meshToTex[i];
		if (materialIndex < textureList.size() && textureList[materialIndex]) {
			textureList[materialIndex]->UseTexture();
		}
		meshList[i]->RenderMesh();
	}
}
void Model::RenderModel(GLuint uniformModelLocation, bool OverrideTexture)
{
	for (size_t i = 0; i < meshList.size(); i++) {
		if (!OverrideTexture) {
			unsigned int materialIndex = meshToTex[i];
			if (materialIndex < textureList.size() && textureList[materialIndex]) {
				textureList[materialIndex]->UseTexture(); // Default texture
			}
		}
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

void Model::LoadMaterials(const aiScene* scene)
{
	textureList.resize(scene->mNumMaterials);

	for (size_t i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial* material = scene->mMaterials[i];
		textureList[i] = nullptr;

		if (material->GetTextureCount(aiTextureType_DIFFUSE)) {
			aiString path;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
				std::string texPath = "Textures/" + std::string(path.C_Str());
				auto texture = std::make_unique<Texture>(texPath);
				if (!texture->LoadTexture()) {
					texture = std::make_unique<Texture>("Textures/plain.png");
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
	}
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
	if (meshList.empty())
		return;

	const auto& mesh = meshList[0];  // Process only the first mesh

	const GLfloat* vertices = mesh->mVertices;
	unsigned int numVertices = mesh->mNumOfVertices;
	const unsigned int floatsPerVertex = 8;

	if (!vertices || numVertices < floatsPerVertex || numVertices % floatsPerVertex != 0)
		return;

	unsigned int vertexCount = numVertices / floatsPerVertex;

	glm::vec3 localMin(vertices[0], vertices[1], vertices[2]);
	glm::vec3 localMax = localMin;

	for (unsigned int i = 1; i < vertexCount; ++i) {
		unsigned int offset = i * floatsPerVertex;
		glm::vec3 pos(vertices[offset], vertices[offset + 1], vertices[offset + 2]);

		localMin = glm::min(localMin, pos);
		localMax = glm::max(localMax, pos);
	}

	// Prepare 8 corners of the local bounding box
	glm::vec3 corners[8] = {
		{ localMin.x, localMin.y, localMin.z },
		{ localMax.x, localMin.y, localMin.z },
		{ localMin.x, localMax.y, localMin.z },
		{ localMax.x, localMax.y, localMin.z },
		{ localMin.x, localMin.y, localMax.z },
		{ localMax.x, localMin.y, localMax.z },
		{ localMin.x, localMax.y, localMax.z },
		{ localMax.x, localMax.y, localMax.z }
	};

	// Apply the Model matrix to all corners
	glm::vec3 transformedMin = glm::vec3(model * glm::vec4(corners[0], 1.0f));
	glm::vec3 transformedMax = transformedMin;

	for (int i = 1; i < 8; ++i) {
		glm::vec3 transformed = glm::vec3(model * glm::vec4(corners[i], 1.0f));
		transformedMin = glm::min(transformedMin, transformed);
		transformedMax = glm::max(transformedMax, transformed);
	}

	untransformedBox = BoundingBox{ localMin, localMax };
	box = BoundingBox{ transformedMin, transformedMax };
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


