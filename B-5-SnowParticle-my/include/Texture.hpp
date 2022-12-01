#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <iostream>
#include <string>

#include "log.h"
#include "stb_image.h"

using namespace std;

class CTexture
{
   public:
    GLuint textureID;

    CTexture() {}

    ~CTexture()
    {
        glDeleteTextures(1, &textureID);
    }

    void loadTexture(string path, bool constrain = false)
    {
        glGenTextures(1, &textureID);
        int width, height, nrComponents;
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
        if (data) {
            GLenum format;
            switch (nrComponents) {
                case 1:
                    format = GL_RED;
                    break;
                case 3:
                    format = GL_RGB;
                    break;
                case 4:
                    format = GL_RGBA;
                    break;
            }
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(
                GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
            );
            glGenerateMipmap(GL_TEXTURE_2D);
            // if (!constrain) {
            // 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            // 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            // }
            // else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // }
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            log_info("load texture %s ok", path.c_str());
        } else {
            std::cout << "Texture failed to load at path: " << path << std::endl;
        }
        stbi_image_free(data);
    }
};
