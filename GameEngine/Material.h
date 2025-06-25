#pragma once

#include <GL/glew.h>
#include "Texture.h"
#include "Skybox.h"

class Material {
public:
	Material();
	Material(GLfloat sIntensity, GLfloat shine);

	void UseMaterial(GLuint specularIntensityLocation, GLuint shininessLocation);
	void UseMaterial(GLuint specularMapLocation, GLuint usesSpecularMap, 
		GLuint reflectivityLocation, GLuint skyboxLocation, GLuint usesReflections);
	void SetSpecularMap(Texture* texture) { specularMap = texture; hasSpecularMap = (specularMap != nullptr); }
	~Material();

private:
	GLfloat specularIntensity;
	GLfloat shininess;

	Texture* specularMap;
	bool hasSpecularMap;

	Skybox* skybox;
	GLfloat reflectivity = 1.0f;
	bool usesReflections = true;
};

