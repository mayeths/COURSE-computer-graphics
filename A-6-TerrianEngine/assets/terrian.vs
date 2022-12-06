#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

uniform sampler2D heightMap;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;
out float Height;

void main() {
    float height = texture(heightMap, aTex).y * 32.0; // RGBA get G (gray picture all same)
    gl_Position = projection * view * model * vec4(aPos.x, height, aPos.z, 1.0);
    Height = (model * vec4(aPos.x, height, aPos.z, 1.0)).y;
    // gl_Position = projection * view * model * vec4(aPos.x, aPos.y * 100, aPos.z, 1.0);
    TexCoord = aTex;
}
