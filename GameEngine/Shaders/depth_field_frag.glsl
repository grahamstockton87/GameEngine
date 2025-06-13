#version 330 core

out vec4 FragColor;
in  vec2 TexCoords;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;

// DOF parameters
uniform float focalDistance;
uniform float focalRange;
uniform float maxBlur;
uniform vec2  texelSize;

// linearize [0–1] depth to view-space Z
float LinearizeDepth(float z) {
    float near = 0.1;
    float far  = 100.0;
    float z_ndc = z * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z_ndc * (far - near));
}

// compute circle-of-confusion in pixels
float ComputeCoC(float viewZ) {
    float d   = viewZ - focalDistance;
    float coc = clamp(abs(d) / focalRange, 0.0, 1.0);
    return coc * maxBlur;
}

void main() {
    // 1) fetch sharp color + raw depth
    vec3 sharpColor = texture(sceneColor, TexCoords).rgb;
    float rawDepth  = texture(sceneDepth, TexCoords).r;

    // 2) turn that into a blur radius
    float viewZ  = LinearizeDepth(rawDepth);
    //float radius = ComputeCoC(viewZ);
    float radius = 20;
    // DEBUG: visualize normalized blur radius
    #ifdef DEBUG_SHOW_RADIUS
        float g = clamp(radius / maxBlur, 0.0, 1.0);
        FragColor = vec4(vec3(g), 1.0);
        return;
    #endif

    // 3) if almost in focus, just output sharp
    if (radius < 0.5) {
        FragColor = vec4(sharpColor, 1.0);
        return;
    }

    // 4) box-blur over ⌊radius⌋ pixels
    int N      = int(floor(radius));
    vec3 sum   = vec3(0.0);
    float cnt  = 0.0;
    for (int x = -N; x <= N; ++x) {
        for (int y = -N; y <= N; ++y) {
            vec2 off = vec2(float(x), float(y)) * texelSize;
            sum += texture(sceneColor, TexCoords + off).rgb;
            cnt += 1.0;
        }
    }
    vec3 blurColor = sum / cnt;

    // 5) blend based on fractional part of radius
    float w = fract(radius);
    vec3 finalColor = mix(blurColor, sharpColor, 1.0 - w);

    FragColor = vec4(1.0);
}
