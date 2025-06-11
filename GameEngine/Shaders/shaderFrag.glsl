#version 330 core

in vec2  TexCoord;
in vec3  Normal;
in vec3  FragPos;
in vec4  DirectionalLightSpacePos;

out vec4 color;

// ---- CONFIG ----
const int MAX_POINT_LIGHTS = 3;
const int MAX_SPOT_LIGHTS  = 3;
const int DIR_PCF_TAPS     = 4;   // 2×2 PCF
const int OMNI_SAMPLES     = 8;   // fewer cubemap samples

// ---- STRUCTS & UNIFORMS (unchanged) ----
struct Light { vec3 color; float ambientIntensity, diffuseIntensity; };
struct DirectionalLight { Light base; vec3 direction; };
struct PointLight { Light base; vec3 position; float constant, linear, exponent; };
struct SpotLight  { PointLight base; vec3 direction; float edge; };
struct OmniShadowMap { samplerCube shadowMap; float farPlane; };
struct Material { float specularIntensity, shininess; };

uniform int               pointLightCount;
uniform int               spotLightCount;
uniform DirectionalLight  directionalLight;
uniform PointLight        pointLights[MAX_POINT_LIGHTS];
uniform SpotLight         spotLights[MAX_SPOT_LIGHTS];
uniform OmniShadowMap     omniShadowMaps[MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS];

uniform sampler2D         theTexture;
uniform sampler2D         directionalShadowMap;
uniform Material          material;
uniform vec3              eyePosition;

// Precomputed disk offsets (shrunken) for cubemap PCF
const vec3 disk[OMNI_SAMPLES] = vec3[](
    vec3(1,1,0), vec3(-1,1,0), vec3(1,-1,0), vec3(-1,-1,0),
    vec3(1,0,1), vec3(-1,0,1), vec3(1,0,-1), vec3(-1,0,-1)
);

// ---- HELPERS ----

// 2×2 PCF for directional shadow
float CalcDirShadow() {
    vec3 proj = DirectionalLightSpacePos.xyz / DirectionalLightSpacePos.w;
    proj = proj * 0.5 + 0.5;
    if (proj.z > 1.0) return 0.0;

    float bias = max(0.005 * (1.0 - dot(normalize(Normal), normalize(directionalLight.direction))), 0.0005);
    vec2 texel = 1.0 / vec2(textureSize(directionalShadowMap,0));
    float depth = proj.z - bias;
    float visibility = 0.0;

    // four taps instead of nine
    visibility += depth > texture(directionalShadowMap, proj.xy + vec2(0,0)       * texel).r ? 1.0 : 0.0;
    visibility += depth > texture(directionalShadowMap, proj.xy + vec2(1,0)       * texel).r ? 1.0 : 0.0;
    visibility += depth > texture(directionalShadowMap, proj.xy + vec2(0,1)       * texel).r ? 1.0 : 0.0;
    visibility += depth > texture(directionalShadowMap, proj.xy + vec2(1,1)       * texel).r ? 1.0 : 0.0;

    return visibility / float(DIR_PCF_TAPS);
}

// fewer omni shadow samples
float CalcOmniShadow(PointLight light, int idx) {
    vec3 L = FragPos - light.position;
    float currentDepth = length(L);
    float bias = 0.05;
    float viewDist = length(eyePosition - FragPos);
    float radius = (viewDist / omniShadowMaps[idx].farPlane) * 0.1;

    float shadow = 0.0;
    for (int i = 0; i < OMNI_SAMPLES; ++i) {
        float closest = texture(omniShadowMaps[idx].shadowMap, L + disk[i]*radius).r
                        * omniShadowMaps[idx].farPlane;
        shadow += (currentDepth - bias > closest) ? 1.0 : 0.0;
    }
    return shadow / float(OMNI_SAMPLES);
}

// single lighting function (ambient+diffuse+spec)
vec3 CalcSingleLight(Light light, vec3 L, float shadowFactor) {
    vec3 N = normalize(Normal) * (gl_FrontFacing ? 1.0 : -1.0);
    vec3 V = normalize(eyePosition - FragPos);
    vec3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0);
    vec3 ambient = light.color * light.ambientIntensity;
    vec3 diffuse = light.color * light.diffuseIntensity * diff;
    vec3 specular = vec3(0.0);

    if (diff > 0.0) {
        float spec = pow(max(dot(V, R), 0.0), material.shininess);
        specular = light.color * material.specularIntensity * spec;
    }

    return ambient + (1.0 - shadowFactor) * (diffuse + specular);
}

// ---- MAIN ----
void main() {
    vec4 tex = texture(theTexture, TexCoord);
    if (tex.a < 0.01) discard;

    // pre‐cache common values
    vec3 N = normalize(Normal) * (gl_FrontFacing ? 1.0 : -1.0);
    vec3 V = normalize(eyePosition - FragPos);

    // 1) Directional
    float dirShadow   = CalcDirShadow();
    vec3  dirLight    = normalize(directionalLight.direction);
    vec3  resultColor = CalcSingleLight(directionalLight.base, -dirLight, dirShadow);

    // 2) Point lights
    for (int i = 0; i < MAX_POINT_LIGHTS; ++i) {
        if (i >= pointLightCount) break;
        PointLight pl = pointLights[i];
        vec3 L = normalize(FragPos - pl.position);
        float att = pl.constant + pl.linear * length(FragPos - pl.position)
                  + pl.exponent * pow(length(FragPos - pl.position), 2.0);
        float sh = CalcOmniShadow(pl, i);
        resultColor += CalcSingleLight(pl.base, L, sh) / att;
    }

    // 3) Spot lights
    for (int i = 0; i < MAX_SPOT_LIGHTS; ++i) {
        if (i >= spotLightCount) break;
        SpotLight sl = spotLights[i];
        vec3 L  = normalize(FragPos - sl.base.position);
        float theta = dot(L, normalize(sl.direction));
        if (theta > sl.edge) {
            float falloff = (theta - sl.edge) / (1.0 - sl.edge);
            float att     = sl.base.constant + sl.base.linear * length(FragPos - sl.base.position)
                          + sl.base.exponent * pow(length(FragPos - sl.base.position), 2.0);
            float sh      = CalcOmniShadow(sl.base, pointLightCount + i);
            resultColor  += falloff * (CalcSingleLight(sl.base.base, L, sh) / att);
        }
    }

    // finalize
    resultColor *= tex.a;
    color = vec4(tex.rgb * resultColor, tex.a);
}
