// fullscreen_quad.vert
#version 330 core

layout(location = 0) in vec2 aPos;        // e.g. (-1,-1),(+1,-1),(+1,+1),(-1,+1)
layout(location = 1) in vec2 aTexCoords;  // e.g. (0,0),(1,0),(1,1),(0,1)

out vec2 TexCoord;

void main() {
    TexCoord    = aTexCoords;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
