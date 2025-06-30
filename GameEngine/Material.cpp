#include "Material.h"
#include <iostream>

Material::Material()
{
	specularIntensity = 128.0f;
	shininess = 128.0f;
	usesReflections = true;
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
		glActiveTexture(GL_TEXTURE0 + 3);
		glBindTexture(GL_TEXTURE_2D, specularMap->GetTextureID());
		// turn the map on
		glUniform1i(usesSpecularMapLocation, 1);
	}
	else {
		// turn the map off (shader will skip sampling)
		glUniform1i(usesSpecularMapLocation, 0);
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
