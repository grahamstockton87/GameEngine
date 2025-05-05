#pragma once

#include <string>
#include "CommonValues.h"
#include <GL/glew.h>

class Texture {
public:
	Texture();
	Texture(std::string fileLoc);
	
	bool LoadTexture();
	bool LoadTextureA();

	void UseTexture();
	void ClearTexture();

	~Texture();
private:
	GLuint textureID;
	int width, height, bitDepth;

	std::string fileLocation;
};

