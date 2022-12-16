#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stb_image.h>

#include <iostream>
#include <string>

#include "util/log.h"

using namespace std;

class Texture
{
   public:
    std::string path;
    int width;
    int height;
    int nChannels;
    GLuint ID = 0;

    bool Load2D(string path)
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
        this->Bind();
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        this->Unbind();
        stbi_image_free(data);
        return true;
    }

    void Active(GLenum TEXTURE_ID)
    {
        glActiveTexture(TEXTURE_ID);
        glBindTexture(GL_TEXTURE_2D, this->ID);
    }

    void Bind()
    {
        glBindTexture(GL_TEXTURE_2D, this->ID);
    }

    void Unbind()
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    ~Texture()
    {
        glDeleteTextures(1, &this->ID);
    }

};
