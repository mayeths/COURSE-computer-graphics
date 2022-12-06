#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

// uniform float offsetY;
// uniform int renderFlip;
out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    // if (renderFlip == 0) {
    // } else {
    //     float dist = aPos.y - offsetY;
    //     vec3 newPos = aPos.xyz;
    //     newPos.y = aPos.y - 2 * dist;
    //     gl_Position = vec4(newPos, 1.0);
    // }
    TexCoord = aTex;
}