#version 330 core

in vec3 pos;

out vec4 color;

uniform sampler2D flankTextureID;

void main() {
    vec4 texColor = texture(flankTextureID, gl_PointCoord);
    texColor.a = texColor.a;
    color = texColor;
}
