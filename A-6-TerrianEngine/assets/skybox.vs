#version 330 core
layout (location = 0) in float aTexIndex;
layout (location = 1) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out float TextureIndex;
out mat4 Transform0;
out mat4 Transform1;
out mat4 Transform2;
out mat4 Transform3;
out mat4 Transform4;
out mat4 Transform5;
out float Invalid;

uniform mat4 model;
uniform mat4 model0;
uniform mat4 model1;
uniform mat4 model2;
uniform mat4 model3;
uniform mat4 model4;
uniform mat4 model5;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 transform0;
uniform mat4 transform1;
uniform mat4 transform2;
uniform mat4 transform3;
uniform mat4 transform4;
uniform mat4 transform5;
uniform float offsetY;
uniform int renderFlip;

void main()
{
    int index = int(aTexIndex);
    mat4 realModel;
    if (index < 4)
        if (index < 2)
            if (index == 0) realModel = model0;
            else realModel = model1;
        else
            if (index == 2) realModel = model2;
            else realModel = model3;
    else
        if (index < 6)
            if (index == 4) realModel = model4;
            else realModel = model5;


    // gl_Position = projection * view * model * vec4(aPos, 1.0f);
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    TextureIndex = aTexIndex;

    vec4 localPos = vec4(aPos, 1.0f);
    vec4 modelPos = realModel * localPos;

    if (renderFlip == 0) {
        gl_Position = projection * view * modelPos;
        Invalid = modelPos.y < offsetY ? 1 : 0;
    } else {
        modelPos.y = modelPos.y - 2 * (modelPos.y - offsetY);
        gl_Position = projection * view * modelPos;
        Invalid = modelPos.y > offsetY ? 1 : 0;
    }

    // Transform0 = transform0;
    // Transform1 = transform1;
    // Transform2 = transform2;
    // Transform3 = transform3;
    // Transform4 = transform4;
    // Transform5 = transform5;
}
