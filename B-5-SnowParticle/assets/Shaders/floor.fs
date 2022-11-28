#version 330 core
out vec4 color;

in vec2 TexCoord;
uniform sampler2D floorTexID;

void main(){
    // color = vec4(0.5,0.5,0.5,1.0f);
    color = texture(floorTexID, TexCoord);
}
