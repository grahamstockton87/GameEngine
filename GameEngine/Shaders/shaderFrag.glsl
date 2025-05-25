#version 330

// Inputs from the vertex shader
in vec4 vCol;           // Vertex color (currently unused)
in vec2 TexCoord;       // Texture coordinates
in vec3 Normal;         // Normal at the fragment
in vec3 FragPos;        // World-space position of the fragment
in vec4 DirectionalLightSpacePos;

// Output color to the framebuffer
out vec4 color;

// Maximum light limits
const int MAX_POINT_LIGHTS = 3;
const int MAX_SPOT_LIGHTS = 3;

// Base light structure used for directional and point lights
struct Light {
    vec3 color;              // RGB color of the light
    float ambientIntensity;  // How strong ambient light is
    float diffuseIntensity;  // How strong diffuse light is
};

// Directional light: light with a direction, but no position
struct DirectionalLight {
    Light base;              // Basic light properties
    vec3 direction;          // Light direction in world space
};

// Point light: light that emits in all directions from a point
struct PointLight {
    Light base;              // Basic light properties
    vec3 position;           // Position of the light
    float constant;          // Attenuation: constant factor
    float linear;            // Attenuation: linear factor
    float exponent;          // Attenuation: quadratic factor
};

// Spotlight: point light with a direction and a cutoff angle
struct SpotLight {
    PointLight base;         // Inherits from point light
    vec3 direction;          // Direction the spotlight is facing
    float edge;              // Cosine of spotlight angle (cutoff)
};

struct OmniShadowMap{
    samplerCube shadowMap;
    float farPlane; 
};

// Material properties for specular lighting
struct Material {
    float specularIntensity; // Strength of specular highlights
    float shininess;         // Shininess factor for specular exponent
};

// Uniform inputs from the CPU side
uniform int pointLightCount;                         // Number of point lights
uniform int spotLightCount;                          // Number of spotlights
uniform DirectionalLight directionalLight;           // Directional light uniform
uniform PointLight pointLights[MAX_POINT_LIGHTS];    // Array of point lights
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];       // Array of spotlights
uniform OmniShadowMap omniShadowMaps[MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS];

uniform sampler2D theTexture;                        // 2D texture sampler
uniform sampler2D directionalShadowMap;

uniform Material material;                           // Material uniform
uniform vec3 eyePosition;                            // Camera (eye) position in world space

vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);


float CalcDirectionalShadowFactor(DirectionalLight light){
    vec3 projCoords = DirectionalLightSpacePos.xyz / DirectionalLightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0) return 0.0;

    // Optional bounds check (instead of clamping)
    if(projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;
    vec3 normal = normalize(Normal);
    if (!gl_FrontFacing)
        normal = -normal;

    vec3 lightDir = normalize(light.direction);
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(directionalShadowMap, 0);

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(directionalShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;

    return shadow;
}


float CalcOmniShadowFactor(PointLight light, int shadowIndex){
    vec3 FragToLight = FragPos - light.position;
    float currentDepth = length(FragToLight);

    float shadow = 0.0;
    float bias = 0.05;
    float samples = 20;
    float offset = 0.1;
    float viewDistance = length(eyePosition-FragPos);
    float diskRadius = (1.0 * (viewDistance/omniShadowMaps[shadowIndex].farPlane)) / 25.0;

    for (int i = 0; i < samples; i++){
        float closetDepth = texture(omniShadowMaps[shadowIndex].shadowMap, FragToLight + gridSamplingDisk[i]*diskRadius).r;
        closetDepth *= omniShadowMaps[shadowIndex].farPlane;
        if ( currentDepth - bias > closetDepth){
            shadow += 1.0;
        }
    }
    shadow /= float(samples);
    return shadow;
}
// Calculates ambient, diffuse, and specular lighting from a given direction
vec4 CalcLightByDirection(Light light, vec3 direction, float shadowFactor) {
    // Flip normal if back-facing
    vec3 normal = normalize(Normal);
    if (!gl_FrontFacing)
        normal = -normal;

    // Ambient component
    vec4 ambientcolor = vec4(light.color, 1.0f) * light.ambientIntensity;

    // Diffuse component
    float diffuseFactor = max(dot(normal, normalize(direction)), 0.0f);
    vec4 diffusecolor = vec4(light.color * light.diffuseIntensity * diffuseFactor, 1.0f);

    // Specular component (Phong model)
    vec4 specularcolor = vec4(0, 0, 0, 0);
    if (diffuseFactor > 0.0f) {
        vec3 fragToEye = normalize(eyePosition - FragPos);
        vec3 reflectedVertex = normalize(reflect(direction, normal));

        float specularFactor = dot(fragToEye, reflectedVertex);
        if (specularFactor > 0.0f) {
            specularFactor = pow(specularFactor, material.shininess);
            specularcolor = vec4(light.color * material.specularIntensity * specularFactor, 1.0f);
        }
    }

    return (ambientcolor + (1.0 - shadowFactor) * (diffusecolor + specularcolor));
}


// Calculates lighting from the directional light
vec4 CalcDirectionalLight() {
    float shadowFactor = CalcDirectionalShadowFactor(directionalLight);
    return CalcLightByDirection(directionalLight.base, directionalLight.direction, shadowFactor);
}

// Calculates lighting from a single point light, with attenuation
vec4 CalcPointLight(PointLight pLight, int shadowIndex) {
    vec3 direction = FragPos - pLight.position;      // Direction from light to fragment
    float distance = length(direction);              // Distance to light
    direction = normalize(direction);                // Normalize for lighting calc

    float shadowFactor = CalcOmniShadowFactor(pLight, shadowIndex);
    vec4 color = CalcLightByDirection(pLight.base, direction, shadowFactor); // Base light calculation

    // Calculate attenuation based on distance
    float attenuation = pLight.exponent * distance * distance +
                        pLight.linear * distance +
                        pLight.constant;

    return (color / attenuation); // Final point light color
}

// Calculates lighting from a single spotlight, based on angle
vec4 CalcSpotLight(SpotLight sLight, int shadowIndex) {
    vec3 rayDirection = normalize(FragPos - sLight.base.position);   // Direction from light to fragment
    float slFactor = dot(rayDirection, sLight.direction);            // Angle between spotlight and fragment

    if (slFactor > sLight.edge) {
        vec4 color = CalcPointLight(sLight.base, shadowIndex);                    // Inside spotlight cone
        return color * (1.0f - (1.0f -slFactor)*(1.0f/(1.0f-sLight.edge)));
    } else {
        return vec4(0, 0, 0, 0);                                      // Outside cone → no light
    }
}

// Aggregates all spotlight contributions
vec4 CalcSpotLights() {
    vec4 totalcolor = vec4(0, 0, 0, 0);
    for (int i = 0; i < spotLightCount; i++) {
        totalcolor += CalcSpotLight(spotLights[i], i + pointLightCount);
    }
    return totalcolor;
}

// Aggregates all point light contributions
vec4 CalcPointLights() {
    vec4 totalcolor = vec4(0, 0, 0, 0);
    for (int i = 0; i < pointLightCount; i++) {
        totalcolor += CalcPointLight(pointLights[i], i);
    }
    return totalcolor;
}

// Final fragment shader entry point
void main() {
    // Sample the texture color (includes alpha)
    vec4 texColor = texture(theTexture, TexCoord);

    // Discard fully transparent fragments (optional for cutout transparency)
    if (texColor.a < 0.01)
        discard;

    // Compute lighting normally
    vec4 lighting = CalcDirectionalLight();
    lighting += CalcPointLights();
    lighting += CalcSpotLights();

    // Dim the lighting by the texture alpha to simulate light passing through
    lighting.rgb *= texColor.a;

    // Final color: texture color modulated by lighting, preserving original alpha
    color = vec4(texColor.rgb * lighting.rgb, texColor.a);
}


