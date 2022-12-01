#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <iostream>
#include <string>

#include "log.h"
#include "stb_image.h"

using namespace std;

class Texture
{
   public:
    std::string path;
    int width;
    int height;
    int nChannels;
    GLuint ID;

    bool Load(string path, GLint wrap_param = GL_REPEAT)
    {
        this->path = path;
        glGenTextures(1, &this->ID);
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &nChannels, 0);
        if (!data) {
            log_error("Texture failed to load: %s", path.c_str());
            return false;
        }
        GLenum format =
            (nChannels == 1) ? GL_RED :
            (nChannels == 3) ? GL_RGB :
            (nChannels == 4) ? GL_RGBA :
            GL_RGB;
        glBindTexture(GL_TEXTURE_2D, this->ID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_param);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_param);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
        return true;
    }

    ~Texture()
    {
        glDeleteTextures(1, &this->ID);
    }

};
