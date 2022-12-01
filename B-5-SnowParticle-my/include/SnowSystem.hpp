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

static const GLfloat PARTICLE_TYPE_LAUNCHER = 0.0f; // See shader
static const GLfloat MAX_VELOCITY = 3.0;
static const GLfloat MIN_VELOCITY = 1.0;
static const GLfloat MAX_LAUNCH = 1.0f * 1000.0f;
static const GLfloat MIN_LAUNCH = 0.5f * 1000.0f;
static const GLfloat INIT_SNOW_SIZE = 10.0f;
static const GLfloat MAX_SIZE = 10.0f;
static const GLfloat MIN_SIZE = 3.0f;
static const GLfloat LOWEST_ALIVE_Y = -100.0f;
static const GLfloat HIGHEST_ALIVE_Y = 200.0f;
static const GLfloat SNOWING_AREA_WIDTH = 500.0f;

static const int MAX_PARTICLES = 40000;
static const int INIT_PARTICLES = 1000;
static const int NUM_RANDOM_TEXTURE = 512;

struct SnowParticle {
    float type;
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetimeMills;
    float size;
};

class SnowSystem : public DrawableObject
{
    bool firstUpdate = true;
    float mTimer = 0;
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
        this->updateIndex = 0;
        this->renderIndex = 1;
        this->firstUpdate = true;
        updateShader.Setup();
        renderShader.Setup();

        SetupRandomTexture(NUM_RANDOM_TEXTURE);
        flakeTexture.load(this->texturePath);

        renderShader.use();
        renderShader.setInt("snowflower", 0);

        SnowParticle particles[MAX_PARTICLES];
        memset(particles, 0, sizeof(particles));
        GenInitLocation(particles, INIT_PARTICLES);
        glGenTransformFeedbacks(2, this->TFO);
        glGenBuffers(2, this->PBO);
        glGenVertexArrays(2, this->PAO);
        for (int i = 0; i < 2; i++) {
            glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, this->TFO[i]);
            glBindBuffer(GL_ARRAY_BUFFER, this->PBO[i]);
            glBindVertexArray(this->PAO[i]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(particles), particles, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, this->PBO[i]);
        }
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
        glBindVertexArray(0);
        updateShader.use();
        updateShader.setInt("gRandomTexture", 0);
        updateShader.setFloat("MAX_SIZE", MAX_SIZE);
        updateShader.setFloat("MIN_SIZE", MIN_SIZE);
        updateShader.setFloat("MAX_LAUNCH", MAX_LAUNCH);
        updateShader.setFloat("MIN_LAUNCH", MIN_LAUNCH);
        glUseProgram(0);
    }

    virtual void update(double now, double deltaUpdateTime)
    {
        mTimer = now * 1000.0f;
        updateShader.use();
        updateShader.setFloat("gDeltaTimeMillis", deltaUpdateTime * 1000.0);
        updateShader.setFloat("gTime", mTimer);
        updateShader.setFloat("floorY", LOWEST_ALIVE_Y);
        glEnable(GL_RASTERIZER_DISCARD);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_1D, randomTexture);
        glBindVertexArray(this->PAO[this->updateIndex]);
        glBindBuffer(GL_ARRAY_BUFFER, this->PBO[this->updateIndex]);
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, this->TFO[this->renderIndex]);

        glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, type));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, position));
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, velocity));
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, lifetimeMills));
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, size));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);

        glBeginTransformFeedback(GL_POINTS);
        if (this->firstUpdate) {
            glDrawArrays(GL_POINTS, 0, INIT_PARTICLES);
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
        glEnable(GL_POINT_SPRITE);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        renderShader.use();
        renderShader.setMat4("model", glm::mat4(1.0f));
        renderShader.setMat4("view", view);
        renderShader.setMat4("projection", projection);
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

    void SetupRandomTexture(unsigned int num)
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

    void GenInitLocation(SnowParticle particles[], int nums)
    {
        for (int x = 0; x < nums; x++) {
            particles[x].type = PARTICLE_TYPE_LAUNCHER;
            particles[x].position = glm::vec3(
                randf32(-SNOWING_AREA_WIDTH, SNOWING_AREA_WIDTH),
                randf32(LOWEST_ALIVE_Y, HIGHEST_ALIVE_Y),
                randf32(-SNOWING_AREA_WIDTH, SNOWING_AREA_WIDTH)
            );
            particles[x].velocity = glm::vec3(0, randf32(MIN_VELOCITY, MAX_VELOCITY), 0);
            particles[x].size = INIT_SNOW_SIZE;
            particles[x].lifetimeMills = randf32(0.1, 0.6);
        }
    }

};
