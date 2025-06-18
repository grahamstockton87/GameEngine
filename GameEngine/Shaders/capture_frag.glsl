#version 330 core
out vec4 FragColor;

in vec3 WorldDir;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183); // = (1 / (2π), 1 / π)

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    uv.y = 1.0 - uv.y; // Flip vertically
    return uv;
}

void main()
{
    vec3 dir = normalize(WorldDir);
    vec2 uv = SampleSphericalMap(dir);
    vec3 color = texture(equirectangularMap, uv).rgb;
    FragColor = vec4(color, 1.0);
}
