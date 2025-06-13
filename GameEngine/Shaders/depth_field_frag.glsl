#version 330 core

out vec4 FragColor;
in  vec2 TexCoords;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;
uniform float focalRange;
uniform float maxBlur;
uniform vec2  texelSize;

float LinearizeDepth(float z) {
    float near = 0.1;
    float far  = 100.0;
    float z_ndc = z * 2.0 - 1.0;
    return (2.0 * near * far)
         / (far + near - z_ndc * (far - near));
}

// CoC now takes both your pixel’s depth and the focus depth
float ComputeCoC(float depth, float focusZ) {
    float d = depth - focusZ;
    float coc = clamp(abs(d) / focalRange, 0.0, 1.0);
    return coc * maxBlur;
}

void main() {
    // 1) fetch raw depths
    float rawThis   = texture(sceneDepth, TexCoords).r;
    float rawCenter = texture(sceneDepth, vec2(0.5, 0.5)).r;

    // 2) linearize both
    float viewZ     = LinearizeDepth(rawThis);
    float focusZ    = LinearizeDepth(rawCenter);

    // 3) compute blur radius relative to center‐depth
    float radius = ComputeCoC(viewZ, focusZ);

    // 4) very simple box blur (you can re-introduce your blend later)
    int   N    = int(floor(radius));
    vec3  sum  = vec3(0.0);
    float cnt  = 0.0;
    for (int x = -N; x <= N; ++x) {
      for (int y = -N; y <= N; ++y) {
        vec2 off = vec2(x, y) * texelSize;
        sum += texture(sceneColor, TexCoords + off).rgb;
        cnt += 1.0;
      }
    }
    vec3 blurColor = (cnt > 0.0) ? sum / cnt : texture(sceneColor, TexCoords).rgb;

    FragColor = vec4(blurColor, 1.0);
}
