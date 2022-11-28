#pragma once

#include "DrawableObject.hpp"
#include "Shader.hpp"

// https://community.khronos.org/t/how-to-make-array-of-sampler2d-uniforms/52862/3
// https://stackoverflow.com/questions/12372058/how-to-use-gl-texture-2d-array-in-opengl-3-2
// https://www.khronos.org/opengl/wiki/Array_Texture
// https://stackoverflow.com/questions/73546924/how-to-use-gl-texture-2d-array-with-stb-image

class SkyBox : public DrawableObject
{
public:
    Shader shader;
    std::string imagePaths[6]; /* Top, North, East, South, West */
    GLuint textureIDs[6];
    GLuint VAO = 0, VBO = 0, EBO = 0;
    std::array<GLfloat, 6 * (6 * 6)> vertices;
    GLfloat imageRotation[6];
    GLfloat waterRoll;

    void SetTopImagePath(const std::string path, GLfloat rotation = 0)
    {
        this->imagePaths[0] = path;
        this->imageRotation[0] = rotation;
    }
    void SetNorthImagePath(const std::string path, GLfloat rotation = 0)
    {
        this->imagePaths[1] = path;
        this->imageRotation[1] = rotation;
    }
    void SetEastImagePath(const std::string path, GLfloat rotation = 0)
    {
        this->imagePaths[2] = path;
        this->imageRotation[2] = rotation;
    }
    void SetSouthImagePath(const std::string path, GLfloat rotation = 0)
    {
        this->imagePaths[3] = path;
        this->imageRotation[3] = rotation;
    }
    void SetWestImagePath(const std::string path, GLfloat rotation = 0)
    {
        this->imagePaths[4] = path;
        this->imageRotation[4] = rotation;
    }
    void SetBottomImagePath(const std::string path, GLfloat rotation = 0)
    {
        this->imagePaths[5] = path;
        this->imageRotation[5] = rotation;
    }
    void SetShaderPath(const std::string vertexPath, const std::string fragmentPath)
    {
        this->shader.vertexPath = vertexPath;
        this->shader.fragmentPath = fragmentPath;
    }
    void SetBoxWidth(const GLfloat width)
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
            // Right (x=0.5 and yz plane same) 这里的texture coords是1-x（所以是100011对比上面的011100）因为为了翻转图像。我不知道比较方便的翻转图像的方法，所以只能这样了。y坐标还是001110。
            2, +0.5f * width, -0.5f * width, -0.5f * width,  1.0f, 0.0f,
            2, +0.5f * width, +0.5f * width, -0.5f * width,  0.0f, 0.0f,
            2, +0.5f * width, +0.5f * width, +0.5f * width,  0.0f, 1.0f,
            2, +0.5f * width, +0.5f * width, +0.5f * width,  0.0f, 1.0f,
            2, +0.5f * width, -0.5f * width, +0.5f * width,  1.0f, 1.0f,
            2, +0.5f * width, -0.5f * width, -0.5f * width,  1.0f, 0.0f,
            // Front (z=0.5 and xy plane same) 这里的texture coords是1-x（所以是100011对比上面的011100）因为为了翻转图像。我不知道比较方便的翻转图像的方法，所以只能这样了。y坐标还是001110。
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
            // Bottom (y=-0.5 and xz plane same)
            5, -0.5f * width, -0.5f * width, -0.5f * width,  0.0f * width, 0.0f * width,
            5, +0.5f * width, -0.5f * width, -0.5f * width,  1.0f * width, 0.0f * width,
            5, +0.5f * width, -0.5f * width, +0.5f * width,  1.0f * width, 1.0f * width,
            5, +0.5f * width, -0.5f * width, +0.5f * width,  1.0f * width, 1.0f * width,
            5, -0.5f * width, -0.5f * width, +0.5f * width,  0.0f * width, 1.0f * width,
            5, -0.5f * width, -0.5f * width, -0.5f * width,  0.0f * width, 0.0f * width,
        };

        if (this->VAO != 0 && this->VBO != 0) {
            glBindVertexArray(this->VAO);
            glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
            glBufferData(GL_ARRAY_BUFFER,
                sizeof(GLfloat) * this->vertices.size(),
                this->vertices.data(),
                GL_DYNAMIC_DRAW
            );
        }
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
        glGenTextures(6, this->textureIDs);
        stbi_set_flip_vertically_on_load(true);
        for (int i = 0; i < 6; i++) {
            glBindTexture(GL_TEXTURE_2D, this->textureIDs[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            if (i != 5) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }
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
        this->shader.setInt("textureID5", 5);
        this->waterRoll = 0.0;

        for (int i = 0; i < 6; i++) {
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
        glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, this->textureIDs[5]);

        Scene scene;
        // model = glm::rotate(model, this->imageRotation[0], scene.Down());

        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_BACK);
        // glFrontFace(GL_CW);

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
        this->shader.setMat4("model5", glm::rotate(model, glm::radians(this->imageRotation[5]), scene.Down()));
        this->shader.setFloat("WaterRoll", this->waterRoll);
        this->waterRoll += deltaRenderTime;
        glDrawArrays(GL_TRIANGLES, 0, 30);

        // glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        // glEnable(GL_ALPHA_TEST);
        // glAlphaFunc(GL_EQUAL, 1.0);
        // glBindTexture(GL_TEXTURE_2D, texture[5]);   //绘制半透明海面波浪
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    //用于绘制半透明的海面波纹以与底面纹理形成天空倒影的效果
        glDrawArrays(GL_TRIANGLES, 30, 6);
        glDisable(GL_BLEND);
        // glEnable(GL_DEPTH_TEST);

        // glDisable(GL_CULL_FACE);
    }

    virtual void GUIcallback(double lastRenderTime, double now) {}

};
