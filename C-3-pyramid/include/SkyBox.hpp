#pragma once

#include "DrawableObject.hpp"
#include "Shader.hpp"

// https://community.khronos.org/t/how-to-make-array-of-sampler2d-uniforms/52862/3
// https://stackoverflow.com/questions/12372058/how-to-use-gl-texture-2d-array-in-opengl-3-2
// https://www.khronos.org/opengl/wiki/Array_Texture
// https://stackoverflow.com/questions/73546924/how-to-use-gl-texture-2d-array-with-stb-image

class SkyBox : public DrawableObject<SkyBox>
{
public:
    Shader shader;
    std::string imagePaths[5]; /* Top, North, East, South, West */
    GLuint textureIDs[5];
    GLuint VAO = 0, VBO = 0, EBO = 0;
    std::array<GLfloat, 5 * (6 * 6)> vertices;
    GLfloat imageRotation[5];

    SkyBox& SetTopImagePath(const std::string path, GLfloat rotation = 0)
    {
        this->imagePaths[0] = path;
        this->imageRotation[0] = rotation;
        return *this;
    }
    SkyBox& SetNorthImagePath(const std::string path, GLfloat rotation = 0)
    {
        this->imagePaths[1] = path;
        this->imageRotation[1] = rotation;
        return *this;
    }
    SkyBox& SetEastImagePath(const std::string path, GLfloat rotation = 0)
    {
        this->imagePaths[2] = path;
        this->imageRotation[2] = rotation;
        return *this;
    }
    SkyBox& SetSouthImagePath(const std::string path, GLfloat rotation = 0)
    {
        this->imagePaths[3] = path;
        this->imageRotation[3] = rotation;
        return *this;
    }
    SkyBox& SetWestImagePath(const std::string path, GLfloat rotation = 0)
    {
        this->imagePaths[4] = path;
        this->imageRotation[4] = rotation;
        return *this;
    }
    SkyBox& SetShaderPath(const std::string vertexPath, const std::string fragmentPath)
    {
        this->shader.vertexPath = vertexPath;
        this->shader.fragmentPath = fragmentPath;
        return *this;
    }
    SkyBox& SetBoxWidth(const GLfloat width)
    {
        this->vertices = {
            // textureid // positions in local space        // texture coords
            // Top (y=0.5 and xz plane same)
            0, -0.5f * width, +0.5f * width, -0.5f * width,  0.0f, 0.0f,
            0, +0.5f * width, +0.5f * width, -0.5f * width,  1.0f, 0.0f,
            0, +0.5f * width, +0.5f * width, +0.5f * width,  1.0f, 1.0f,
            0, +0.5f * width, +0.5f * width, +0.5f * width,  1.0f, 1.0f,
            0, -0.5f * width, +0.5f * width, +0.5f * width,  0.0f, 1.0f,
            0, -0.5f * width, +0.5f * width, -0.5f * width,  0.0f, 0.0f,
            // Back (z=-0.5 and xy plane same)
            1, -0.5f * width, -0.5f * width, -0.5f * width,  0.0f, 0.0f,
            1, +0.5f * width, -0.5f * width, -0.5f * width,  1.0f, 0.0f,
            1, +0.5f * width, +0.5f * width, -0.5f * width,  1.0f, 1.0f,
            1, +0.5f * width, +0.5f * width, -0.5f * width,  1.0f, 1.0f,
            1, -0.5f * width, +0.5f * width, -0.5f * width,  0.0f, 1.0f,
            1, -0.5f * width, -0.5f * width, -0.5f * width,  0.0f, 0.0f,
            // Right (x=0.5 and yz plane same)
            2, +0.5f * width, -0.5f * width, -0.5f * width,  1.0f, 0.0f,
            2, +0.5f * width, +0.5f * width, -0.5f * width,  0.0f, 0.0f,
            2, +0.5f * width, +0.5f * width, +0.5f * width,  0.0f, 1.0f,
            2, +0.5f * width, +0.5f * width, +0.5f * width,  0.0f, 1.0f,
            2, +0.5f * width, -0.5f * width, +0.5f * width,  1.0f, 1.0f,
            2, +0.5f * width, -0.5f * width, -0.5f * width,  1.0f, 0.0f,
            // Front (z=0.5 and xy plane same)
            3, -0.5f * width, -0.5f * width, +0.5f * width,  1.0f, 0.0f,
            3, +0.5f * width, -0.5f * width, +0.5f * width,  0.0f, 0.0f,
            3, +0.5f * width, +0.5f * width, +0.5f * width,  0.0f, 1.0f,
            3, +0.5f * width, +0.5f * width, +0.5f * width,  0.0f, 1.0f,
            3, -0.5f * width, +0.5f * width, +0.5f * width,  1.0f, 1.0f,
            3, -0.5f * width, -0.5f * width, +0.5f * width,  1.0f, 0.0f,
            // Left (x=-0.5 and yz plane same)
            4, -0.5f * width, -0.5f * width, -0.5f * width,  0.0f, 0.0f,
            4, -0.5f * width, +0.5f * width, -0.5f * width,  1.0f, 0.0f,
            4, -0.5f * width, +0.5f * width, +0.5f * width,  1.0f, 1.0f,
            4, -0.5f * width, +0.5f * width, +0.5f * width,  1.0f, 1.0f,
            4, -0.5f * width, -0.5f * width, +0.5f * width,  0.0f, 1.0f,
            4, -0.5f * width, -0.5f * width, -0.5f * width,  0.0f, 0.0f,
        };

        // image rotation
        GLfloat cs[5], sn[5];
        for (int i = 0; i < 5; i++) {
            GLfloat theta = glm::radians(this->imageRotation[i]);
            cs[i] = cos(theta);
            sn[i] = sin(theta);
        }

        if (this->VAO != 0 && this->VBO != 0) {
            glBindVertexArray(this->VAO);
            glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
            glBufferData(GL_ARRAY_BUFFER,
                sizeof(GLfloat) * this->vertices.size(),
                this->vertices.data(),
                GL_DYNAMIC_DRAW
            );
        }
        return *this;
    }

    void Setup()
    {
        // Vertex
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(GLfloat) * this->vertices.size(),
            this->vertices.data(),
            GL_DYNAMIC_DRAW
        );

        glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (void*)(1*sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (void*)(4*sizeof(GLfloat)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Shader
        this->shader.Setup();
        glGenTextures(5, this->textureIDs);
        stbi_set_flip_vertically_on_load(true);
        for (int i = 0; i < 5; i++) {
            glBindTexture(GL_TEXTURE_2D, this->textureIDs[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            const char *path = this->imagePaths[i].c_str();
            int width, height, nchannels;
            GLubyte *data = stbi_load(path, &width, &height, &nchannels, 0);
            if (!data) {
                log_error("Failed to load texture %s", path);
                continue;
            }
            GLenum format = nchannels == 4 ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(data);
        }
        this->shader.use();
        this->shader.setInt("textureID0", 0);
        this->shader.setInt("textureID1", 1);
        this->shader.setInt("textureID2", 2);
        this->shader.setInt("textureID3", 3);
        this->shader.setInt("textureID4", 4);

        for (int i = 0; i < 5; i++) {
            glm::mat4 trans = glm::mat4(1.0f);
            trans = glm::rotate(trans, glm::radians(this->imageRotation[i]), glm::vec3(0.0, 0.0, 1.0));
            // trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));
            std::string name = "transform" + std::to_string(i);
            this->shader.setMat4(name.c_str(), trans);
        }
    }

    virtual void update(double now, double deltaUpdateTime) {}

    virtual void render(double now, double deltaRenderTime, const glm::mat4 &view, const glm::mat4 &projection) {

        glBindVertexArray(this->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, this->textureIDs[0]);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, this->textureIDs[1]);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, this->textureIDs[2]);
        glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, this->textureIDs[3]);
        glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, this->textureIDs[4]);

        Scene scene;
        // model = glm::rotate(model, this->imageRotation[0], scene.Down());

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, this->position);
        this->shader.use();
        this->shader.setMat4("view", view);
        this->shader.setMat4("projection", projection);
        this->shader.setMat4("model", model);
        this->shader.setMat4("model0", glm::rotate(model, glm::radians(this->imageRotation[0]), scene.Down()));
        this->shader.setMat4("model1", glm::rotate(model, glm::radians(this->imageRotation[1]), scene.Behind()));
        this->shader.setMat4("model2", glm::rotate(model, glm::radians(this->imageRotation[2]), scene.Right()));
        this->shader.setMat4("model3", glm::rotate(model, glm::radians(this->imageRotation[3]), scene.Behind()));
        this->shader.setMat4("model4", glm::rotate(model, glm::radians(this->imageRotation[4]), scene.Right()));
        glDrawArrays(GL_TRIANGLES, 0, 30);
    }

    virtual void GUIcallback() {}

};
