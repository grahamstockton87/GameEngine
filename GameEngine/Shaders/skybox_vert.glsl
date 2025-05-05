#version 330

// Input data
layout (location = 0) in vec3 pos;    // Vertex position

out vec3 TexCoords;

uniform mat4 projection;   
uniform mat4 view;

void main(){
    TexCoords = pos;
    gl_Position = projection * view * vec4(pos, 1.0);
}