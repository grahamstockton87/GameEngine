#ifndef GAMEENGINE_TEXTURE_H
#define GAMEENGINE_TEXTURE_H

#include "stb_image.h"

#include <string>
#include <GL/glew.h>

class Texture {
public:
	Texture();
	Texture(std::string fileLoc);

	Texture(const Texture& other) = default; // Disable copy constructor

	
	bool LoadTexture();
	bool LoadTextureA();

	void UseTexture();
	void UseTexture(GLenum textureUnit);
	void ClearTexture();
	GLuint GetTextureID() const { return textureID; }

	~Texture();
private:
	GLuint textureID;

	int width, height, bitDepth;

	std::string fileLocation;
};

#endif // !GAMEENGINE_TEXTURE_H