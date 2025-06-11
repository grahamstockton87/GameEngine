// dof.frag
#version 330 core

in vec2  TexCoord;
out vec4 FragColor;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;

// DOF parameters
uniform float focalDistance;   // in world units
uniform float focalRange;      // half‐width of focus band
uniform float maxBlur;         // max blur radius in pixels
uniform vec2  texelSize;       // = 1.0/(screenW,screenH)

float LinearizeDepth(float z) {
    float near = 0.1;
    float far  = 100.0;
    return (2.0 * near) / (far + near - z * (far - near));
}

float ComputeCoC(float depth) {
    float d   = depth - focalDistance;
    float coc = clamp(abs(d) / focalRange, 0.0, 1.0);
    return coc * maxBlur;
}

void main() {
    float nonLinZ  = texture(sceneDepth, TexCoord).r;
    float linearZ  = LinearizeDepth(nonLinZ);
    float radius   = ComputeCoC(linearZ);

    if (radius < 1.0) {
        FragColor = texture(sceneColor, TexCoord);
        return;
    }

    int   N      = int(floor(radius));
    vec3  accum  = vec3(0.0);
    float weight = 0.0;

    for (int x = -N; x <= N; ++x) {
        for (int y = -N; y <= N; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            accum       += texture(sceneColor, TexCoord + offset).rgb;
            weight      += 1.0;
        }
    }

    vec3 colorBlur = accum / weight;
    FragColor = vec4(colorBlur, 1.0);
}
