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

void Material::UseMaterial(GLuint specularMapLocation, GLuint usesSpecularMap, GLuint reflectivityLocation, GLuint skyboxLocation, GLuint usesReflectionsLocation)
{
	if (specularMap) {
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, specularMap->GetTextureID());
		glUniform1i(specularMapLocation, 3);
	}
	glUniform1i(usesSpecularMap, hasSpecularMap);

	glUniform1i(skyboxLocation, 5);
	glUniform1i(reflectivityLocation, reflectivity);
	glUniform1i(usesReflectionsLocation, usesReflections);
}


Material::~Material()
{
}
