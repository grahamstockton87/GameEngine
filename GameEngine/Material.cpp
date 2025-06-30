#include "Material.h"
#include <iostream>

Material::Material()
{
	specularIntensity = 128.0f;
	shininess = 128.0f;
	usesReflections = false;
}

Material::Material(GLfloat sIntensity, GLfloat shine)
{
	specularIntensity = sIntensity;
	shininess = shine;
}

void Material::UseMaterial(GLuint specularIntensityLocation, GLuint shininessLocation)
{
	glUniform1f(specularIntensityLocation, specularIntensity);
	glUniform1f(shininessLocation, shininess);
}

void Material::UseMaterial(
	GLuint specularMapLocation,
	GLuint usesSpecularMapLocation,
	GLuint reflectivityLocation,
	GLuint usesReflectionsLocation,
	GLuint skyboxLocation,
	GLuint skyboxTextureID)
{
	// — specular map —
	if (usesSpecularMap && specularMap->GetTextureID() != 0) {
		// bind the texture object to unit 3
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, specularMap->GetTextureID());
		// turn the map on
		glUniform1i(specularMapLocation, 2);
		glUniform1i(usesSpecularMapLocation, 3);
	}
	else {
		// turn the map off (shader will skip sampling)
		glUniform1i(usesSpecularMapLocation, 0);
		std::cout << "No specular map set for material!" << specularMap->GetTextureID() << std::endl;
	}
	if (diffuseMap) {
		diffuseMap->UseTexture();
	}
	else {
		std::cout << "No diffuse map set for material!" << std::endl;
	}
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
	glUniform1i(skyboxLocation, 5);

	glUniform1i(usesReflectionsLocation, usesReflections);
	glUniform1f(reflectivityLocation, reflectivity);
}



Material::~Material()
{
}
