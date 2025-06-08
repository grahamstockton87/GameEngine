
#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include "Shader.h"

namespace RenderUtils{
    void DrawFullScreenQuad(Shader& shader,
    GLuint tex0, const char* uni0,
    GLuint tex1 = 0, const char* uni1 = nullptr,
    float  blend = 0.0f);
}

