#version 330 core

in vec2  TexCoord;
out vec4 FragColor;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;

// DOF parameters
uniform float focalDistance;   // in world units
uniform float focalRange;      // half‐width of focus band
uniform float maxBlur;         // max blur radius in pixels

// Must be set from your app: 1.0 / (screenWidth, screenHeight)
uniform vec2  texelSize;

// Linearize non‐linear depth
float LinearizeDepth(float z) {
    float near = 0.1;
    float far  = 100.0;
    return (2.0 * near) / (far + near - z * (far - near));
}

// Compute blur radius (in pixels) from depth
float ComputeCoC(float depth) {
    float d   = depth - focalDistance;
    float coc = clamp(abs(d) / focalRange, 0.0, 1.0);
    return coc * maxBlur;
}

void main() {
    // 1 read & linearize depth
    float nonLinZ = texture(sceneDepth, TexCoord).r;
    float linearZ = LinearizeDepth(nonLinZ);

    // 2 compute blur radius
    float radius = ComputeCoC(linearZ);

    // 3 if almost in focus, skip blur
    if (radius < 1.0) {
        FragColor = texture(sceneColor, TexCoord);
        return;
    }

    // 4 box‐blur over a (2N+1)×(2N+1) kernel
    int   N      = int(floor(radius));
    vec3  accum  = vec3(0.0);
    float weight = 0.0;

    // sample in a grid centered on TexCoord
    for (int x = -N; x <= N; ++x) {
      for (int y = -N; y <= N; ++y) {
        vec2 offset = vec2(float(x), float(y)) * texelSize;
        float w      = 1.0;               // uniform weight
        accum       += texture(sceneColor, TexCoord + offset).rgb * w;
        weight      += w;
      }
    }

    // 5 average & output
    vec3 colorBlur = accum / weight;
    FragColor = vec4(colorBlur, 1.0);
}
