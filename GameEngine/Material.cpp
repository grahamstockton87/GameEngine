#include "Material.h"

Material::Material()
{
	specularIntensity = 0.0f;
	shininess = 0.0f;
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
	if (specularMap) {
		glActiveTexture(GL_TEXTURE0 + 3);
		glBindTexture(GL_TEXTURE_2D, specularMap->GetTextureID());
		glUniform1i(specularMapLocation, 3);
		glUniform1i(usesSpecularMapLocation, 1);
	}
	else {
		glUniform1i(usesSpecularMapLocation, 0);
	}
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
	glUniform1i(skyboxLocation, 5);
	glUniform1i(usesReflectionsLocation, 1);

	glUniform1f(reflectivityLocation, reflectivity);
}



Material::~Material()
{
}
