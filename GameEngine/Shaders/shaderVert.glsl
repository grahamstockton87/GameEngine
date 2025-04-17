#version 330 core

// Input data
layout (location = 0) in vec3 pos;    // Vertex position
layout (location = 1) in vec2 tex;    // Texture coordinates
layout (location = 2) in vec3 norm;   // Normal

// Output data
out vec4 vCol;        
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

// Uniforms for transformations
uniform mat4 model;        
uniform mat4 projection;   
uniform mat4 view;

void main() {
    // Transform vertex position
    gl_Position = projection * view * model * vec4(pos, 1.0);

    // Pass data to fragment shader
    vCol = vec4(clamp(pos, 0.0f, 1.0f), 1.0f);  // Color based on position (just for visualization)
    TexCoord = tex;
    Normal = mat3(transpose(inverse(model))) * norm;  // Normal transformation
    FragPos = (model * vec4(pos, 1.0)).xyz;  // Fragment position in world space
}
