// skybox.vert
#version 330 core
layout(location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;        // make sure this has no translation

void main() {
    TexCoords = aPos;
    vec4 clip = projection * view * vec4(aPos, 1.0);
    // push to far plane:
    gl_Position = clip.xyww;
}
