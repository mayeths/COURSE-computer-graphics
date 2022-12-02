#pragma once

#include "DrawableObject.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

class Terrian : public DrawableObject
{
    uint64_t nvertices = 0;
public:
    const GLuint NUM_PATCH_PTS = 4;
    const GLuint rez = 20;

    Shader shader;
    std::string heightMapPath;
    std::string defaultTexturePath;
    std::string detailTexturePath;
    Texture heightMapTexture;
    Texture defaultTexture;
    Texture detailTexture;
    GLuint VAO = 0, VBO = 0;
    bool polygonMode = false;

    void SetShaderPath(const std::string vertexPath, const std::string fragmentPath, const std::string tessControlPath, const std::string tessEvalPath)
    {
        this->shader.vertexPath = vertexPath;
        this->shader.fragmentPath = fragmentPath;
        this->shader.tessControlPath = tessControlPath;
        this->shader.tessEvalPath = tessEvalPath;
    }
    void SetHeightMapPath(const std::string heightMapPath)
    {
        this->heightMapPath = heightMapPath;
    }
    void SetTexturePath(const std::string defaultTexturePath)
    {
        this->defaultTexturePath = defaultTexturePath;
    }
    void SetDetailTexturePath(const std::string detailTexturePath)
    {
        this->detailTexturePath = detailTexturePath;
    }

    void Setup()
    {
        // load and create a texture
        // -------------------------
        this->shader.Setup();

        this->heightMapTexture.Load2D(this->heightMapPath.c_str());
        this->heightMapTexture.Bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        this->heightMapTexture.Unbind();

        detailTexture.Load2D(this->detailTexturePath.c_str());
        detailTexture.Bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        detailTexture.Unbind();

        defaultTexture.Load2D(this->defaultTexturePath.c_str());
        defaultTexture.Bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        defaultTexture.Unbind();

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        std::vector<GLfloat> vertices;

        int width = this->heightMapTexture.width;
        int height = this->heightMapTexture.height;
        GLfloat frez = (GLfloat)rez;
        for(GLuint i = 0; i <= rez-1; i++) {
            for(GLuint j = 0; j <= rez-1; j++) {
                vertices.push_back(-width/2.0f + width*i/frez); // v.x
                vertices.push_back(0.0f); // v.y
                vertices.push_back(-height/2.0f + height*j/frez); // v.z
                vertices.push_back(i / frez); // u
                vertices.push_back(j / frez); // v

                vertices.push_back(-width/2.0f + width*(i+1)/frez); // v.x
                vertices.push_back(0.0f); // v.y
                vertices.push_back(-height/2.0f + height*j/frez); // v.z
                vertices.push_back((i+1) / frez); // u
                vertices.push_back(j / frez); // v

                vertices.push_back(-width/2.0f + width*i/frez); // v.x
                vertices.push_back(0.0f); // v.y
                vertices.push_back(-height/2.0f + height*(j+1)/frez); // v.z
                vertices.push_back(i / frez); // u
                vertices.push_back((j+1) / frez); // v

                vertices.push_back(-width/2.0f + width*(i+1)/frez); // v.x
                vertices.push_back(0.0f); // v.y
                vertices.push_back(-height/2.0f + height*(j+1)/frez); // v.z
                vertices.push_back((i+1) / frez); // u
                vertices.push_back((j+1) / frez); // v
            }
        }
        this->nvertices = (uint64_t)rez * (uint64_t)rez * 4;
        log_debug("Loaded %llu patches of 4 control points each", (uint64_t)(rez * rez));
        log_debug("Processing %llu vertices in vertex shader", this->nvertices);
        log_debug("Loaded heightmap of size [%d, %d]", height, width);

        // first, configure the cube's VAO (and VBO)
        glGenVertexArrays(1, &this->VAO);
        glBindVertexArray(this->VAO);

        glGenBuffers(1, &this->VBO);
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // texCoord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(1);

        glPatchParameteri(GL_PATCH_VERTICES, this->NUM_PATCH_PTS);

    }

    virtual void update(double now, double deltaUpdateTime) {}

    virtual void render(double now, double deltaRenderTime, const glm::mat4 &view, const glm::mat4 &projection)
    {
        this->heightMapTexture.Active(GL_TEXTURE0);
        this->defaultTexture.Active(GL_TEXTURE1);
        this->shader.use();
        this->shader.setInt("heightMap", 0);
        this->shader.setInt("texture_terrianID", 1);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, this->position);
        this->shader.setMat4("projection", projection);
        this->shader.setMat4("view", view);
        this->shader.setMat4("model", model);

        // render the terrain
        glBindVertexArray(this->VAO);
        GLint previousPolygonMode[2];
        if (this->polygonMode) {
            glGetIntegerv(GL_POLYGON_MODE, previousPolygonMode);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        glDrawArrays(GL_PATCHES, 0, this->NUM_PATCH_PTS * this->rez * this->rez);
        if (this->polygonMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            // glPolygonMode(previousPolygonMode[0], previousPolygonMode[1]);
        }
    }

    virtual void GUIcallback(double lastRenderTime, double now)
    {
        ImGui::SetNextWindowPos(ImVec2(120, 44), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::Begin("Terrian", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        ImGui::Text("Terrian Vertex Num: %llu", nvertices);
        ImGui::End();
    }
};
