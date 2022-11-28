#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
// const highp vec3 W = vec3(0.2125, 0.7154, 0.0721);

void main()
{
    // vec3 col = texture(screenTexture, TexCoords).rgb;
    // FragColor = vec4(col, 1.0);
    FragColor = texture(screenTexture, TexCoords);

    // gray method 0 (LearnOpenGL)
    // float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
    // FragColor = vec4(average, average, average, 1.0);

    // https://juejin.cn/post/6865856200198914061
    // method 1
    // FragColor = texture(screenTexture, TexCoords);
    // float luminance = dot(FragColor.rgb, W);
    // FragColor = vec4(vec3(luminance), 1.0);
    // method 2
    // FragColor = texture(screenTexture, TexCoords);
    // FragColor = vec4(FragColor.g, FragColor.g, FragColor.g, 1.0);
} 