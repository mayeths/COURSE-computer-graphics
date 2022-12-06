#version 410 core

in float Height;
in float Invalid;
in vec3 TerrianTexCoord;

uniform float offsetY;
uniform int renderFlip;
uniform sampler2D defaultTextureID;
uniform sampler2D detailTextureID;
uniform float detailScale; // 越大detail的纹理越密

out vec4 FragColor;

void main()
{
    float h = (Height + 16)/64.0f;
    vec2 TerrianTexCoord2D = vec2(TerrianTexCoord.x, TerrianTexCoord.y);
    if (Invalid == 1)
        discard;
    // if (renderFlip == 0) {
    //     if (Height < offsetY)
    //         discard;
    // } else {
    //     if (Height >= offsetY)
    //         discard;
    // }
    vec4 macro = texture(defaultTextureID, TerrianTexCoord2D);
    vec4 micro = texture(detailTextureID, detailScale * TerrianTexCoord2D);
    FragColor = macro + micro - 0.5f;
    // FragColor = vec4(h, h, h, 1.0);
}