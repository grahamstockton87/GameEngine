#version 330

// Input data
layout (location = 0) in vec3 pos;    // Vertex position

uniform mat4 model;

void main(){
    gl_Position = model * vec4(pos, 1.0);
}