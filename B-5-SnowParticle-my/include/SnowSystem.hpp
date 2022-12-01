#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "DrawableObject.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "random.h"
#include "log.h"

#define PARTICLE_TYPE_BASE 0.0f // See shader update.gs

struct SnowParticle {
    float type = PARTICLE_TYPE_BASE;
    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::vec3 velocity = glm::vec3(0, 0, 0);
    float age = 0;
    float size = 0;
};

class SnowSystem : public DrawableObject
{
    static inline const GLfloat MAX_VELOCITY = 50.0;
    static inline const GLfloat MIN_VELOCITY = 10.0;
    static inline const GLfloat MAX_LAUNCH = 1.0f * 1000.0f;
    static inline const GLfloat MIN_LAUNCH = 0.5f * 1000.0f;
    static inline const GLfloat INIT_SNOW_SIZE = 10.0f;
    static inline const GLfloat MAX_SIZE = 10.0f;
    static inline const GLfloat MIN_SIZE = 3.0f;
    static inline const GLfloat LOWEST_ALIVE_Y = -100.0f;
    static inline const GLfloat HIGHEST_ALIVE_Y = 200.0f;
    static inline const GLfloat SNOWING_AREA_WIDTH = 500.0f;

    static inline const int INIT_NUM_PARTICLES = 1000;
    static inline const int MAX_NUM_PARTICLES = 100000;
    static inline const int NUM_RANDOM_TEXTURE = 512;

    bool firstUpdate = true;
public:
    GLuint updateIndex, renderIndex;
    GLuint PAO[2]; // ParticleBufferArrayObject
    GLuint PBO[2]; // ParticleBufferObject
    GLuint TFO[2]; // TransformBufferObject
    GLuint randomTexture;
    Texture flakeTexture;
    Shader updateShader;
    Shader renderShader;
    std::string texturePath;

    SnowSystem()
    {
    }

    ~SnowSystem()
    {
    }

    void SetTexturePath(const std::string texturePath)
    {
        this->texturePath = texturePath;
    }

    void SetRenderShader(const std::string vertexPath, const std::string fragmentPath)
    {
        this->renderShader.vertexPath = vertexPath;
        this->renderShader.fragmentPath = fragmentPath;
    }

    void SetUpdateShader(const std::string vertexPath, const std::string fragmentPath, const std::string geometryPath, const std::vector<const GLchar *> varyings)
    {
        this->updateShader.vertexPath = vertexPath;
        this->updateShader.fragmentPath = fragmentPath;
        this->updateShader.geometryPath = geometryPath;
        this->updateShader.varyings = varyings;
    }

    void Setup()
    {
        flakeTexture.load(this->texturePath);

        updateShader.Setup();
        renderShader.Setup();
        renderShader.use();
        renderShader.setInt("flankTextureID", 0);

        this->SetupRandomTexture(NUM_RANDOM_TEXTURE);

        std::vector<SnowParticle> particles(MAX_NUM_PARTICLES);
        for (int i = 0; i < INIT_NUM_PARTICLES; i++) {
            particles[i].type = PARTICLE_TYPE_BASE;
            particles[i].position = glm::vec3(
                randf32(-SNOWING_AREA_WIDTH, SNOWING_AREA_WIDTH),
                randf32(LOWEST_ALIVE_Y, HIGHEST_ALIVE_Y),
                randf32(-SNOWING_AREA_WIDTH, SNOWING_AREA_WIDTH)
            );
            particles[i].velocity = glm::vec3(0, randf32(MIN_VELOCITY, MAX_VELOCITY), 0);
            particles[i].size = INIT_SNOW_SIZE;
            particles[i].age = randf32(0.1, 0.6);
        }

        glGenTransformFeedbacks(2, this->TFO);
        glGenBuffers(2, this->PBO);
        glGenVertexArrays(2, this->PAO);
        for (int i = 0; i < 2; i++) {
            glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, this->TFO[i]);
            glBindBuffer(GL_ARRAY_BUFFER, this->PBO[i]);
            glBindVertexArray(this->PAO[i]);
            glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(SnowParticle), particles.data(), GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, this->PBO[i]);
        }
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
        glBindVertexArray(0);
        updateShader.use();
        updateShader.setInt("randomTextureID", 0);
        updateShader.setFloat("MAX_SIZE", MAX_SIZE);
        updateShader.setFloat("MIN_SIZE", MIN_SIZE);
        updateShader.setFloat("MAX_LAUNCH", MAX_LAUNCH);
        updateShader.setFloat("MIN_LAUNCH", MIN_LAUNCH);
        glUseProgram(0);

        this->updateIndex = 0;
        this->renderIndex = 1;
        this->firstUpdate = true;
    }

    virtual void update(double now, double deltaUpdateTime)
    {
        updateShader.use();
        updateShader.setFloat("deltaUpdateTime", deltaUpdateTime);
        updateShader.setFloat("now", now);
        updateShader.setFloat("LOWEST_ALIVE_Y", LOWEST_ALIVE_Y);
        glEnable(GL_RASTERIZER_DISCARD);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_1D, randomTexture);
        glBindVertexArray(this->PAO[this->updateIndex]);
        glBindBuffer(GL_ARRAY_BUFFER, this->PBO[this->updateIndex]);
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, this->TFO[this->renderIndex]);

        glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, type));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, position));
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, velocity));
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, age));
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, size));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);

        glBeginTransformFeedback(GL_POINTS);
        if (this->firstUpdate) {
            glDrawArrays(GL_POINTS, 0, INIT_NUM_PARTICLES);
            this->firstUpdate = false;
        } else {
            glDrawTransformFeedback(GL_POINTS, this->TFO[this->updateIndex]);
        }
        glEndTransformFeedback();
        glDisableVertexAttribArray(4);
        glDisableVertexAttribArray(3);
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glDisable(GL_RASTERIZER_DISCARD);
    }

    virtual void render(double now, double deltaRenderTime, const glm::mat4 &view, const glm::mat4 &projection)
    {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_POINT_SPRITE);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        renderShader.use();
        renderShader.setMat4("model", glm::mat4(1.0f));
        renderShader.setMat4("view", view);
        renderShader.setMat4("projection", projection);

        glBindVertexArray(this->PAO[this->renderIndex]);
        glBindBuffer(GL_ARRAY_BUFFER, this->PBO[this->renderIndex]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, position));
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, size));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, flakeTexture.ID);
        glDrawTransformFeedback(GL_POINTS, this->TFO[this->renderIndex]);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glEnable(GL_DEPTH_TEST);

        this->updateIndex = this->renderIndex;
        this->renderIndex = (this->renderIndex + 1) & 0x1;
    }

    void SetupRandomTexture(GLuint num)
    {
        std::vector<glm::vec3> textures(num);
        for (int i = 0; i < num; i++) {
            textures[i] = glm::vec3(randf32(0, 1), randf32(0, 1), randf32(0, 1));
        }
        glGenTextures(1, &randomTexture);
        glBindTexture(GL_TEXTURE_1D, randomTexture);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, num, 0, GL_RGB, GL_FLOAT, textures.data());
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

};
