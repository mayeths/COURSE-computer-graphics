#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "DrawableObject.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "log.h"

static const GLfloat PARTICLE_TYPE_LAUNCHER = 0.0f;
static const GLfloat PARTICLE_TYPE_SHELL = 1.0f;
static const glm::vec3 MAX_VELOC = glm::vec3(0.0,-3.0,0.0);
static const glm::vec3 MIN_VELOC = glm::vec3(0.0,-1.0,0.0);
static const GLfloat MAX_LAUNCH = 1.0f * 1000.0f;
static const GLfloat MIN_LAUNCH = 0.5f * 1000.0f;
static const GLfloat INIT_SIZE = 10.0f;
static const GLfloat MAX_SIZE = 10.0f;
static const GLfloat MIN_SIZE = 3.0f;
static const GLfloat ground = 3.0f;

static const int MAX_PARTICLES = 5000;
static const int INIT_PARTICLES = 500;
static const glm::vec3 center = glm::vec3(0.0f);
static const float areaLength = 500.0f;
static const float fallHeight = 180.0f;

struct SnowParticle {
    float type;
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetimeMills;
    float size;
};

class SnowSystem : public DrawableObject
{
public:
    SnowSystem();
    ~SnowSystem();
    virtual void update(double now, double deltaUpdateTime)
    {
        mTimer = now * 1000.0f;
        UpdateParticles(deltaUpdateTime * 1000.0f, ground);
    }

    virtual void render(double now, double deltaRenderTime, const glm::mat4 &view, const glm::mat4 &projection)
    {
        RenderParticles(glm::mat4(1.0f), view, projection);
        mCurVBOIndex = mCurTransformFeedbackIndex;
        mCurTransformFeedbackIndex = (mCurTransformFeedbackIndex + 1) & 0x1;
    }

private:
    bool InitSnow();
    void UpdateParticles(float frametimeMills, float floorY);//更新粒子的位置等
    void InitRandomTexture(unsigned int size);//生成1维随机纹理
    void RenderParticles(glm::mat4 worldMatrix, glm::mat4 viewMatrix, glm::mat4 projectMatrix);
    void GenInitLocation(SnowParticle partciles[], int nums);//生成初始粒子

    unsigned int mCurVBOIndex, mCurTransformFeedbackIndex;
    GLuint mParticleBuffers[2]; //粒子发射系统的两个顶点缓存区
    GLuint mParticleArrays[2];
    GLuint mTransformFeedbacks[2];//粒子发射系统对应的TransformFeedback
    GLuint mRandomTexture;//随机一维纹理
    Texture mSparkTexture;//Alpha纹理
    Texture mStartTexture;
    float mTimer;//粒子发射器已经发射的时间
    bool mFirst;
    Shader* updateShader;
    Shader* renderShader;

};

SnowSystem::SnowSystem()
{
    mCurVBOIndex = 0;
    mCurTransformFeedbackIndex = 1;
    mFirst = true;
    mTimer = 0;
    const GLchar* varyings[5] = {"Type1","Position1", "Velocity1","Age1","Size1"};
    updateShader = new Shader("./assets/Shaders/Update.vs", "./assets/Shaders/Update.fs", 
                    "./assets/Shaders/Update.gs", varyings, 5);
    renderShader = new Shader("./assets/Shaders/Render.vs", "./assets/Shaders/Render.fs");
    InitRandomTexture(512);
    mSparkTexture.Load("./assets/Textures/snowstorm.bmp");
    renderShader->use();
    renderShader->setInt("snowflower", 0);
    glUseProgram(0);
    InitSnow();
}


SnowSystem::~SnowSystem()
{
}

bool SnowSystem::InitSnow()
{
    SnowParticle particles[MAX_PARTICLES];
    memset(particles, 0, sizeof(particles));
    GenInitLocation(particles, INIT_PARTICLES);
    glGenTransformFeedbacks(2, mTransformFeedbacks);
    glGenBuffers(2, mParticleBuffers);
    glGenVertexArrays(2, mParticleArrays);
    for (int i = 0; i < 2; i++)
    {
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedbacks[i]);
        glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffers[i]);
        glBindVertexArray(mParticleArrays[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(particles), particles, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mParticleBuffers[i]);
        //glBindBuffer(GL_ARRAY_BUFFER,0);
    }
    //glBindTransformFeedback(GL_TRANSFORM_FEEDBACK,0);
    glBindVertexArray(0);
    //绑定纹理
    updateShader->use();
    updateShader->setInt("gRandomTexture", 0);
    updateShader->setFloat("MAX_SIZE", MAX_SIZE);
    updateShader->setFloat("MIN_SIZE", MIN_SIZE);
    updateShader->setFloat("MAX_LAUNCH", MAX_LAUNCH);
    updateShader->setFloat("MIN_LAUNCH", MIN_LAUNCH);
    glUseProgram(0);
    return true;
}

void SnowSystem::UpdateParticles(float frametimeMills, float floorY)
{
    updateShader->use();
    updateShader->setFloat("gDeltaTimeMillis", frametimeMills);
    updateShader->setFloat("gTime", mTimer);
    updateShader->setFloat("floorY", floorY);
    //绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, mRandomTexture);

    glEnable(GL_RASTERIZER_DISCARD);//我们渲染到TransformFeedback缓存中去，并不需要光栅化
    glBindVertexArray(mParticleArrays[mCurVBOIndex]);
    glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffers[mCurVBOIndex]);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedbacks[mCurTransformFeedbackIndex]);

    glEnableVertexAttribArray(0);//type
    glEnableVertexAttribArray(1);//position
    glEnableVertexAttribArray(2);//velocity
    glEnableVertexAttribArray(3);//lifetime
    glEnableVertexAttribArray(4);//size
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, type));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, position));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, velocity));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, lifetimeMills));
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, size));
    glBeginTransformFeedback(GL_POINTS);
    if (mFirst)
    {
        glDrawArrays(GL_POINTS, 0, INIT_PARTICLES);
        mFirst = false;
    }
    else {
        glDrawTransformFeedback(GL_POINTS, mTransformFeedbacks[mCurVBOIndex]);
    }
    glEndTransformFeedback();
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisable(GL_RASTERIZER_DISCARD);
    //glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SnowSystem::RenderParticles(glm::mat4 worldMatrix,
    glm::mat4 viewMatrix, glm::mat4 projectMatrix)
{
    glEnable(GL_POINT_SPRITE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    renderShader->use();
    renderShader->setMat4("model", worldMatrix);
    renderShader->setMat4("view", viewMatrix);
    renderShader->setMat4("projection", projectMatrix);
    //glBindVertexArray(mParticleArrays[mCurTransformFeedbackIndex]);
    //glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER,0,mParticleBuffers[mCurTransformFeedbackIndex]);
    glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffers[mCurTransformFeedbackIndex]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, position));
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, size));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mSparkTexture.ID);
    glDrawTransformFeedback(GL_POINTS, mTransformFeedbacks[mCurTransformFeedbackIndex]);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnable(GL_DEPTH_TEST);
}

void SnowSystem::InitRandomTexture(unsigned int size)
{
    srand(time(NULL));
    glm::vec3* pRandomData = new glm::vec3[size];
    for (int i = 0; i < size; i++)
    {
        pRandomData[i].x = float(rand()) / float(RAND_MAX);
        pRandomData[i].y = float(rand()) / float(RAND_MAX);
        pRandomData[i].z = float(rand()) / float(RAND_MAX);
    }
    glGenTextures(1, &mRandomTexture);
    glBindTexture(GL_TEXTURE_1D, mRandomTexture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, size, 0, GL_RGB, GL_FLOAT, pRandomData);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    delete[] pRandomData;
    pRandomData = nullptr;
}

void SnowSystem::GenInitLocation(SnowParticle particles[], int nums)
{
    srand(time(NULL));
    for (int x = 0; x < nums; x++) {
        glm::vec3 record(0.0f);
        record.x = (2.0f*float(rand()) / float(RAND_MAX) - 1.0f)*areaLength;
        record.z = (2.0f*float(rand()) / float(RAND_MAX) - 1.0f)*areaLength;
        record.y = fallHeight;
        particles[x].type = PARTICLE_TYPE_LAUNCHER;
        particles[x].position = record;
        particles[x].velocity = (MAX_VELOC - MIN_VELOC)*(float(rand()) / float(RAND_MAX))
            + MIN_VELOC;//在最大最小速度之间随机选择
        particles[x].size = INIT_SIZE;//发射器粒子大小
        particles[x].lifetimeMills = 0.5f*(float(rand()) / float(RAND_MAX)) + 0.1f;
    }
}
