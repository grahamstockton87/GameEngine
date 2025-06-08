#ifndef SKYBOX_H
#define SKYBOX_H



#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Shader.h"
#include "Mesh.h"
#include "CommonValues.h"

//#include "stbi_image_include.cpp"

class Skybox {
public:
	Skybox();

	Skybox(std::vector<std::string> faceLocations);
	void DrawSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
	~Skybox();

private:
	Mesh* skyMesh;
	Shader* skyShader;

	GLuint textureID;
	GLuint uniformProjection, uniformView;
};

#endif // !SKYBOX_H