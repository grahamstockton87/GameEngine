#version 330 core

in vec2  TexCoord;
in vec3  Normal;
in vec3  FragPos;
in vec4  DirectionalLightSpacePos;

out vec4 color;

// ---- CONFIG ----
const int MAX_POINT_LIGHTS = 3;
const int MAX_SPOT_LIGHTS  = 3;
const int DIR_PCF_TAPS     = 4;
const int OMNI_SAMPLES     = 8;

// ---- STRUCTS & UNIFORMS ----
struct Light     { vec3 color; float ambientIntensity, diffuseIntensity; };
struct DirectionalLight { Light base; vec3 direction; };
struct PointLight       { Light base; vec3 position; float constant, linear, exponent; };
struct SpotLight        { PointLight base; vec3 direction; float edge; };
struct OmniShadowMap    { samplerCube shadowMap; float farPlane; };
struct Material         { float specularIntensity, shininess; };

uniform int               pointLightCount;
uniform int               spotLightCount;
uniform DirectionalLight  directionalLight;
uniform PointLight        pointLights[MAX_POINT_LIGHTS];
uniform SpotLight         spotLights[MAX_SPOT_LIGHTS];
uniform OmniShadowMap     omniShadowMaps[MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS];

uniform sampler2D         theTexture;
uniform sampler2D         specularMap;
uniform sampler2D         directionalShadowMap;
uniform samplerCube       skybox;  
uniform Material          material;
uniform vec3              eyePosition;

uniform bool              useSpecularMap;
uniform float             reflectivity;
uniform bool              useReflectivity;

const vec3 disk[OMNI_SAMPLES] = vec3[](
    vec3(1,1,0), vec3(-1,1,0), vec3(1,-1,0), vec3(-1,-1,0),
    vec3(1,0,1), vec3(-1,0,1), vec3(1,0,-1), vec3(-1,0,-1)
);

// ---- HELPERS ----

float CalcDirShadow() {
    vec3 proj = DirectionalLightSpacePos.xyz / DirectionalLightSpacePos.w;
    proj = proj * 0.5 + 0.5;
    if (proj.z > 1.0) return 0.0;

    vec3 N = normalize(Normal) * (gl_FrontFacing ? 1.0 : -1.0);
    float bias = max(0.005 * (1.0 - dot(N, normalize(directionalLight.direction))), 0.0005);

    vec2 texelSize = 1.0 / vec2(textureSize(directionalShadowMap, 0));
    float depth = proj.z - bias;
    float vis = 0.0;
    vis += depth > texture(directionalShadowMap, proj.xy + texelSize * vec2(0,0)).r ? 1.0 : 0.0;
    vis += depth > texture(directionalShadowMap, proj.xy + texelSize * vec2(1,0)).r ? 1.0 : 0.0;
    vis += depth > texture(directionalShadowMap, proj.xy + texelSize * vec2(0,1)).r ? 1.0 : 0.0;
    vis += depth > texture(directionalShadowMap, proj.xy + texelSize * vec2(1,1)).r ? 1.0 : 0.0;

    return vis / float(DIR_PCF_TAPS);
}

float CalcOmniShadow(PointLight light, int idx) {
    vec3 L = FragPos - light.position;
    float currentDepth = length(L);
    float bias = 0.05;
    float viewDist = length(eyePosition - FragPos);
    float radius = (viewDist / omniShadowMaps[idx].farPlane) * 0.1;

    float shadow = 0.0;
    for (int i = 0; i < OMNI_SAMPLES; ++i) {
        float closest = texture(omniShadowMaps[idx].shadowMap, L + disk[i] * radius).r
                      * omniShadowMaps[idx].farPlane;
        shadow += (currentDepth - bias > closest) ? 1.0 : 0.0;
    }
    return shadow / float(OMNI_SAMPLES);
}

vec3 CalcSingleLight(Light light, vec3 L, float shadowFactor) {
    vec3 N = normalize(Normal) * (gl_FrontFacing ? 1.0 : -1.0);
    vec3 V = normalize(eyePosition - FragPos);
    vec3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0);
    vec3 ambient = light.color * light.ambientIntensity;
    vec3 diffuse = light.color * light.diffuseIntensity * diff;
    vec3 specular = vec3(0.0);

    if (diff > 0.0) {
        if (useSpecularMap) {
            float specIntensityFromMap = texture(specularMap, TexCoord).r * 128.0;
            float spec = pow(max(dot(V, R), 0.0), 128.0);
            specular = light.color * spec * specIntensityFromMap;
        } else {
            float spec = pow(max(dot(V, R), 0.0), material.shininess);
            specular = light.color * material.specularIntensity * spec;
        }
    }

    return ambient + (1.0 - shadowFactor) * (diffuse + specular);
}

// ---- MAIN ----
void main() {
    vec4 tex = texture(theTexture, TexCoord);
    if (tex.a < 0.01) discard;

    float dirShadow = CalcDirShadow();
    vec3 dirL = normalize(directionalLight.direction);
    vec3 result = CalcSingleLight(directionalLight.base, -dirL, dirShadow);

    for (int i = 0; i < pointLightCount; ++i) {
        PointLight pl = pointLights[i];
        vec3 L = normalize(FragPos - pl.position);
        float dist = length(FragPos - pl.position);
        float att = pl.constant + pl.linear * dist + pl.exponent * dist * dist;
        float sh = CalcOmniShadow(pl, i);
        result += CalcSingleLight(pl.base, L, sh) / att;
    }

    for (int i = 0; i < spotLightCount; ++i) {
        SpotLight sl = spotLights[i];
        vec3 L = normalize(FragPos - sl.base.position);
        float theta = dot(L, normalize(sl.direction));
        if (theta > sl.edge) {
            float fall = (theta - sl.edge) / (1.0 - sl.edge);
            float dist = length(FragPos - sl.base.position);
            float att = sl.base.constant + sl.base.linear * dist + sl.base.exponent * dist * dist;
            float sh = CalcOmniShadow(sl.base, pointLightCount + i);
            result += fall * (CalcSingleLight(sl.base.base, L, sh) / att);
        }
    }

// --- base lit color (includes texture) ---
    vec3 baseColor = result * tex.rgb;

    // --- compute reflection mask + lookup ---
    vec3 I    = normalize(FragPos - eyePosition);
    vec3 R    = reflect(I, normalize(Normal));
    vec3 env  = texture(skybox, R).rgb;
    vec3 mask = clamp(result, 0.0, 1.0);

    // --- additive reflection ---
    vec3 reflection = env * mask * reflectivity;

    // --- combine ---
    vec3 finalColor = baseColor + reflection;

    // optional tone‐map/clamp so you don’t blow out >1.0
    finalColor = clamp(finalColor, 0.0, 1.0);

    color = vec4(finalColor, tex.a);
}
