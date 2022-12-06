#version 330 core

in vec2 TexCoord;

uniform sampler2D defaultTextureID;

out vec4 Color;
in float Height;

void main() {
    if (Height < 0)
        discard;
    Color = texture(defaultTextureID, TexCoord);
}
