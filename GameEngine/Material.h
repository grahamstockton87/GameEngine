#pragma once

#include <GL/glew.h>
#include "Texture.h"

class Material {
public:
	Material();
	Material(GLfloat sIntensity, GLfloat shine);
	Material(Texture* specularMap, bool usesSpecularMap, GLfloat reflectivity, bool usesReflections);

	void UseMaterial(GLuint specularIntensityLocation, GLuint shininessLocation);
	void UseMaterial(GLuint specularMapLocation, GLuint usesSpecularMapLocation, GLuint reflectivityLocation, GLuint usesReflectionsLocation, GLuint skyboxLocation, GLuint skyboxTextureID);

	void SetSpecularMap(Texture* specularMap){
		this->specularMap = specularMap;
		this->usesSpecularMap = (this->specularMap != nullptr);
	}

	~Material();

private:
	GLfloat specularIntensity;
	GLfloat shininess;
	Texture* specularMap;
	bool usesSpecularMap;
	bool usesReflections;
	GLfloat reflectivity = 0.2f;
};

