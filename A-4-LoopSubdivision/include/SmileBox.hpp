#pragma once

#include "framework/Shader.hpp"
#include "framework/Texture.hpp"
#include "framework/Object.hpp"
#include "util/raii.hpp"

// https://community.khronos.org/t/how-to-make-array-of-sampler2d-uniforms/52862/3
// https://stackoverflow.com/questions/12372058/how-to-use-gl-texture-2d-array-in-opengl-3-2
// https://www.khronos.org/opengl/wiki/Array_Texture
// https://stackoverflow.com/questions/73546924/how-to-use-gl-texture-2d-array-with-stb-image

class SmileBox : public Object
{
public:
    Shader shader;
    std::string texture0Path;
    std::string texture1Path;
    Texture texture0;
    Texture texture1;
    VAO_raii VAO;
    VBO_raii VBO;
    glm::vec3 rotationAxis = glm::vec3(0.0f, 0.0f, -1.0f);
    // GLuint VAO = 0, VBO = 0;
    // GLuint VBO = 0;
    int index;

    void SetShaderPath(const std::string vertexPath, const std::string fragmentPath)
    {
        this->shader.vertexPath = vertexPath;
        this->shader.fragmentPath = fragmentPath;
    }
    void SetTexturePath(const std::string texture0Path, const std::string texture1Path)
    {
        this->texture0Path = texture0Path;
        this->texture1Path = texture1Path;
    }
    void Setup(int index)
    {
        this->index = index;

        // Shader
        this->shader.Setup();

        this->texture0.Load2D(this->texture0Path);
        this->texture0.Bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        this->texture0.Unbind();

        this->texture1.Load2D(this->texture1Path);
        this->texture1.Bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        this->texture1.Unbind();

        GLfloat vertices[] = {
            /** OpenGL is Right-handed system
             * By convention, OpenGL is a right-handed system.
             * What this basically says is that the positive x-axis is to your right, the 
             * positive y-axis is up and the positive z-axis is backwards. Think of your 
             * screen being the center of the 3 axes and the positive z-axis going through 
             * your screen towards you. The axes are drawn as follows:
             * https://learnopengl.com/Getting-started/Coordinate-Systems (#Going 3D)
             * To understand why it's called right-handed do the following:
             * Stretch your right-arm along the positive y-axis with your hand up top.
             * Let your thumb point to the right.
             * Let your pointing finger point up.
             * Now bend your middle finger downwards 90 degrees.
             * If you did things right, your thumb should point towards the positive x-axis, 
             * the pointing finger towards the positive y-axis and your middle finger towards 
             * the positive z-axis. If you were to do this with your left-arm you would see 
             * the z-axis is reversed. This is known as a left-handed system and is commonly 
             * used by DirectX. Note that in normalized device coordinates OpenGL actually 
             * uses a left-handed system (the projection matrix switches the handedness).
             */
            // positions         // texture coords
            // Back (z=-0.5 and xy plane same)
            -0.5f, -0.5f, -0.5f,  0.0f*2, 0.0f*2,
            +0.5f, -0.5f, -0.5f,  1.0f*2, 0.0f*2,
            +0.5f, +0.5f, -0.5f,  1.0f*2, 1.0f*2,
            +0.5f, +0.5f, -0.5f,  1.0f*2, 1.0f*2,
            -0.5f, +0.5f, -0.5f,  0.0f*2, 1.0f*2,
            -0.5f, -0.5f, -0.5f,  0.0f*2, 0.0f*2,
            // Front (z=0.5 and xy plane same)
            -0.5f, -0.5f,  0.5f,  0.0f*2, 0.0f*2,
            +0.5f, -0.5f,  0.5f,  1.0f*2, 0.0f*2,
            +0.5f, +0.5f,  0.5f,  1.0f*2, 1.0f*2,
            +0.5f, +0.5f,  0.5f,  1.0f*2, 1.0f*2,
            -0.5f, +0.5f,  0.5f,  0.0f*2, 1.0f*2,
            -0.5f, -0.5f,  0.5f,  0.0f*2, 0.0f*2,
            // Left (x=-0.5 and yz plane same)
            -0.5f, -0.5f, -0.5f,  0.0f*2, 0.0f*2,
            -0.5f, +0.5f, -0.5f,  1.0f*2, 0.0f*2,
            -0.5f, +0.5f, +0.5f,  1.0f*2, 1.0f*2,
            -0.5f, +0.5f, +0.5f,  1.0f*2, 1.0f*2,
            -0.5f, -0.5f, +0.5f,  0.0f*2, 1.0f*2,
            -0.5f, -0.5f, -0.5f,  0.0f*2, 0.0f*2,
            // Right (x=0.5 and yz plane same)
            +0.5f, -0.5f, -0.5f,  0.0f*2, 0.0f*2,
            +0.5f, +0.5f, -0.5f,  1.0f*2, 0.0f*2,
            +0.5f, +0.5f, +0.5f,  1.0f*2, 1.0f*2,
            +0.5f, +0.5f, +0.5f,  1.0f*2, 1.0f*2,
            +0.5f, -0.5f, +0.5f,  0.0f*2, 1.0f*2,
            +0.5f, -0.5f, -0.5f,  0.0f*2, 0.0f*2,
            // Bottom (y=-0.5 and xz plane same)
            -0.5f, -0.5f, -0.5f,  0.0f*2, 0.0f*2,
            +0.5f, -0.5f, -0.5f,  1.0f*2, 0.0f*2,
            +0.5f, -0.5f, +0.5f,  1.0f*2, 1.0f*2,
            +0.5f, -0.5f, +0.5f,  1.0f*2, 1.0f*2,
            -0.5f, -0.5f, +0.5f,  0.0f*2, 1.0f*2,
            -0.5f, -0.5f, -0.5f,  0.0f*2, 0.0f*2,
            // Top (y=0.5 and xz plane same)
            -0.5f, +0.5f, -0.5f,  0.0f*2, 0.0f*2,
            +0.5f, +0.5f, -0.5f,  1.0f*2, 0.0f*2,
            +0.5f, +0.5f, +0.5f,  1.0f*2, 1.0f*2,
            +0.5f, +0.5f, +0.5f,  1.0f*2, 1.0f*2,
            -0.5f, +0.5f, +0.5f,  0.0f*2, 1.0f*2,
            -0.5f, +0.5f, -0.5f,  0.0f*2, 0.0f*2,
        };

        VAO.create(1);
        VBO.create(1);

        glBindVertexArray(this->VAO.get());
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO.get());
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    virtual void Render(double now, double lastRenderTime, const glm::mat4 &view, const glm::mat4 &projection) {
        this->texture0.Active(GL_TEXTURE0);
        this->texture1.Active(GL_TEXTURE1);
        this->shader.use();
        this->shader.setInt("texture1", 0);
        this->shader.setInt("texture2", 1);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, this->position);
        model = glm::rotate(model, (float)(45 + now * this->index), this->rotationAxis);
        this->shader.setMat4("projection", projection);
        this->shader.setMat4("view", view);
        this->shader.setMat4("model", model);

        glBindVertexArray(this->VAO.get());
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

};
