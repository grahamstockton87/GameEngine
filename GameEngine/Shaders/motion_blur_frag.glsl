#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D currentFrame;
uniform sampler2D previousFrame;
uniform float blendFactor; // e.g., 0.9 for stronger trails

void main()
{
    vec4 curr = texture(currentFrame, TexCoords);
    vec4 prev = texture(previousFrame, TexCoords);
    //FragColor = mix(curr, prev, 0.9); // Blend old and new
    //FragColor = abs(texture(previousFrame, TexCoords) - texture(currentFrame, TexCoords));
    //FragColor = texture(previousFrame, TexCoords); // Should NOT be black anymore
    //FragColor = vec4(1,0,0,1);
    FragColor = mix(curr, prev, blendFactor);
}
                         