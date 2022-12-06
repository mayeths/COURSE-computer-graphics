#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in float TextureIndex;
in float Invalid;
// in mat4 Transform0;
// in mat4 Transform1;
// in mat4 Transform2;
// in mat4 Transform3;
// in mat4 Transform4;
// in mat4 Transform5;

uniform sampler2D textureID0;
uniform sampler2D textureID1;
uniform sampler2D textureID2;
uniform sampler2D textureID3;
uniform sampler2D textureID4;
uniform sampler2D textureID5;
uniform float waterTextureScale; // 越大detail的纹理越密

uniform float WaterRoll;

void main()
{
    float OPACITY = 0.7;
    int index = int(TextureIndex);
    if (Invalid == 1)
        discard;
    if (index < 4)
        if (index < 2)
            if (index == 0) FragColor = texture(textureID0, TexCoord);
            else FragColor = texture(textureID1, TexCoord);
        else
            if (index == 2) FragColor = texture(textureID2, TexCoord);
            else FragColor = texture(textureID3, TexCoord);
    else
        if (index < 6)
            if (index == 4) FragColor = texture(textureID4, TexCoord);
            else FragColor = vec4(1.0, 1.0, 1.0, OPACITY) * texture(textureID5, TexCoord * waterTextureScale + WaterRoll);
}
