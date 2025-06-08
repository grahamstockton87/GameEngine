#pragma once

#include <map>
#include <string>
#include <iostream>
#include <fstream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>

#include "Shader.h"

struct Character {
	GLuint TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	GLuint Advance;
};

class TextRenderer {
public:
	TextRenderer() : textVAO(0), textVBO(0) {}

	std::map<GLchar, Character> GetCharacters() const {
		return Characters;
	}

	bool LoadFont(const std::string& fontPath, unsigned int fontSize = 48) {
		std::ifstream testFont(fontPath);
		if (!testFont.is_open()) {
			std::cerr << "[TextRenderer] Font file not found: " << fontPath << "\n";
			return false;
		}

		FT_Library ft;
		if (FT_Init_FreeType(&ft)) {
			std::cerr << "[TextRenderer] Failed to initialize FreeType.\n";
			return false;
		}

		FT_Face face;
		if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
			std::cerr << "[TextRenderer] Failed to load font: " << fontPath << "\n";
			FT_Done_FreeType(ft);
			return false;
		}

		FT_Set_Pixel_Sizes(face, 0, fontSize);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

		for (GLubyte c = 0; c < 128; ++c) {
			if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
				std::cerr << "[TextRenderer] Failed to load Glyph '" << (char)c << "'\n";
				continue;
			}

			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0, GL_RED, GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				static_cast<GLuint>(face->glyph->advance.x)
			};
			Characters.insert(std::make_pair(c, character));
		}

		std::cout << "[TextRenderer] Loaded " << Characters.size() << " glyphs.\n";

		glBindTexture(GL_TEXTURE_2D, 0);
		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		// Create VAO/VBO
		glGenVertexArrays(1, &textVAO);
		glGenBuffers(1, &textVBO);
		glBindVertexArray(textVAO);
		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		return true;
	}

	void RenderText(Shader& shader, const std::string& text, float x, float y, float scale, glm::vec3 color, const glm::mat4& projection) {
		if (Characters.empty()) {
			std::cerr << "[RenderText] No characters loaded! Did you call LoadFont() after OpenGL init?\n";
			return;
		}

		shader.UseShader();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		glUniform3f(shader.GetTextColorLocation(), color.x, color.y, color.z);
		glUniformMatrix4fv(shader.GetTextProjectionLocation(), 1, GL_FALSE, glm::value_ptr(projection));

		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(textVAO);

		for (const char& c : text) {
			const Character& ch = Characters.at(c);

			float xpos = x + ch.Bearing.x * scale;
			float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			float w = ch.Size.x * scale;
			float h = ch.Size.y * scale;

			float vertices[6][4] = {
				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos,     ypos,       0.0f, 1.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },

				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },
				{ xpos + w, ypos + h,   1.0f, 0.0f }
			};

			glBindTexture(GL_TEXTURE_2D, ch.TextureID);
			glBindBuffer(GL_ARRAY_BUFFER, textVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			x += (ch.Advance >> 6) * scale;
		}

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glEnable(GL_DEPTH_TEST); // Restore state
	}

	void Cleanup() {
		for (auto& pair : Characters)
			glDeleteTextures(1, &pair.second.TextureID);
		Characters.clear();

		if (textVBO) glDeleteBuffers(1, &textVBO);
		if (textVAO) glDeleteVertexArrays(1, &textVAO);
	}

	~TextRenderer() {
		Cleanup();
	}

private:
	std::map<GLchar, Character> Characters;
	GLuint textVAO, textVBO;
};
