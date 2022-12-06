#version 410 core

in float Height;
in vec3 TerrianTexCoord;

uniform sampler2D defaultTextureID;

out vec4 FragColor;

void main()
{
    float h = (Height + 16)/64.0f;
    vec2 TerrianTexCoord2D = vec2(TerrianTexCoord.x, TerrianTexCoord.y);
    if (Height < 2.0)
		discard;
    FragColor = texture(defaultTextureID, TerrianTexCoord2D);
    // FragColor = vec4(h, h, h, 1.0);
}