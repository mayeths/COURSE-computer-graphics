#pragma once

#include <stddef.h>
#include <array>
#include <string>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "raii.hpp"
#include "halfEdge_structure.h"
#include "LoopSubdivision.h"
#include "log.h"
#include "Shader.hpp"
#include "Object/DrawableObject.hpp"

class Mesh : public DrawableObject
{
    guard_t guard;
public:
    Shader shader;
    VAO_raii VAO;
    VBO_raii VBO;
    std::string meshPath;
    std::vector<std::array<double, 3>> vertices;
    std::vector<std::array<int, 3>> faces;
    // uint32_t nvertices = 0, nfaces = 0, count_halfedge_read = 0, count_face_read = 0;
    std::vector<HalfEdge_mesh> meshs;
    int curr = 0;
    int oldKeyQState = GLFW_RELEASE;
    int oldKeyEState = GLFW_RELEASE;
    int nowKeyQState = GLFW_RELEASE;
    int nowKeyEState = GLFW_RELEASE;
    glm::vec4 color = glm::vec4(0, 0, 0, 1.0);

    void SetShaderPath(const std::string vertexPath, const std::string fragmentPath)
    {
        this->shader.vertexPath = vertexPath;
        this->shader.fragmentPath = fragmentPath;
    }
    void SetMeshPath(const std::string &meshPath) {
        this->meshPath = meshPath;
    }

    void Setup() {
        this->shader.Setup();

        std::ifstream fin;
        fin.exceptions(std::ifstream::badbit);
        try {
            fin.open(this->meshPath, std::ios::in);
        } catch (std::ifstream::failure& e) {
            log_error("Mesh(%s) was not successfully read: %s", meshPath.c_str(), e.what());
            return;
        }
        std::string line;
        while (std::getline(fin, line)) {
            if (line.size() == 0) continue;
            std::istringstream instring(line);
            char t;
            instring >> t;
            glm::vec3 data;
            if (t == 'v') {
                std::array<double, 3> positions;
                instring >> positions[0] >> positions[1] >> positions[2];
                this->vertices.push_back(std::move(positions));
            } else if (t == 'f') {
                std::array<int, 3> points;
                instring >> points[0] >> points[1] >> points[2];
                this->faces.push_back(std::move(points));
            }
        }

        this->meshs.resize(1);
        HalfEdge_mesh &mesh = this->meshs[0];

        size_t nvertices = this->vertices.size();
        size_t nfaces = this->faces.size();
        mesh.vertex.resize(nvertices);
        mesh.face.resize(nfaces);
        mesh.halfedge.resize(nfaces * 3);
        for (size_t i = 0; i < nvertices; i++) {
            std::array<double, 3> positions = this->vertices[i];
            HalfEdge_vertex &vertex = mesh.vertex[i];
            vertex.x = positions[0];
            vertex.y = positions[1];
            vertex.z = positions[2];
            vertex.number = i;
        }
        for (size_t i = 0; i < nfaces; i++) {
            std::array<int, 3> points = this->faces[i];
            HalfEdge_vertex &point0 = mesh.vertex[points[0]];
            HalfEdge_vertex &point1 = mesh.vertex[points[1]];
            HalfEdge_vertex &point2 = mesh.vertex[points[2]];

            HalfEdge_face &face = mesh.face[i];
            HalfEdge_halfedge &edge0 = mesh.halfedge[i * 3 + 0];
            HalfEdge_halfedge &edge1 = mesh.halfedge[i * 3 + 1];
            HalfEdge_halfedge &edge2 = mesh.halfedge[i * 3 + 2];

            face.boundary = &edge0;
            edge0.origin = &point0;
            edge1.origin = &point1;
            edge2.origin = &point2;
            if (!point0.beenOrigined) {
                point0.asOrigin = &edge0;
                point0.beenOrigined = true;
            }
            if (!point1.beenOrigined) {
                point1.asOrigin = &edge1;
                point1.beenOrigined = true;
            }
            if (!point2.beenOrigined) {
                point2.asOrigin = &edge2;
                point2.beenOrigined = true;
            }

            edge0.nextEdge = &edge1;
            edge1.nextEdge = &edge2;
            edge2.nextEdge = &edge0;
            edge0.preEdge = &edge2;
            edge1.preEdge = &edge0;
            edge2.preEdge = &edge1;
            edge0.incidentFace = &face;
            edge1.incidentFace = &face;
            edge2.incidentFace = &face;
            face.number = i;
            edge0.number = i * 3 + 0;
            edge1.number = i * 3 + 1;
            edge2.number = i * 3 + 2;
        }

        setNormal(&mesh);
        findTwin(&mesh, nfaces * 3);
        setCrease(&mesh);

        log_debug("Mesh %d vertices %d faces", this->vertices.size(), this->faces.size());
        this->OffloadCurrentMesh();
        this->guard.set();
    }

    void processInput(GLFWwindow *window)
    {
        this->oldKeyQState = this->nowKeyQState;
        this->oldKeyEState = this->nowKeyEState;
        this->nowKeyQState = glfwGetKey(window, GLFW_KEY_Q);
        this->nowKeyEState = glfwGetKey(window, GLFW_KEY_E);
    }

    void TrySwitchMesh(int acc)
    {
        int targetIndex = this->curr + acc;
        if (targetIndex < 0)
            return;

        if (targetIndex >= this->meshs.size()) {
            int oldSize = this->meshs.size();
            int newSize = targetIndex + 1;
            this->meshs.resize(newSize);
            for (int i = oldSize; i < newSize; i++) {
                // generate new mesh
                HalfEdge_mesh &oldMesh = this->meshs[i-1];
                HalfEdge_mesh &newMesh = this->meshs[i];
                newMesh.vertex.reserve(oldMesh.vertex.size() * 8);
                newMesh.face.reserve(oldMesh.face.size() * 8);
                newMesh.halfedge.reserve(oldMesh.halfedge.size() * 8);
                newMesh.normal.reserve(oldMesh.normal.size() * 8);
                loopSubdivision(&oldMesh, &newMesh);
            }
        }
        this->curr = targetIndex;
        this->OffloadCurrentMesh();
    }

    void OffloadCurrentMesh()
    {
        HalfEdge_mesh &mesh = this->meshs[this->curr];
        size_t nfaces = this->faces.size();
        std::vector<GLfloat> fdata;
        for (size_t i = 0; i < nfaces; i++) {
            HalfEdge_face &face = mesh.face[i];
            HalfEdge_halfedge &edge0 = *face.boundary;
            HalfEdge_halfedge &edge1 = *edge0.nextEdge;
            HalfEdge_halfedge &edge2 = *edge0.preEdge;
            HalfEdge_vertex &point0 = *edge0.origin;
            HalfEdge_vertex &point1 = *edge1.origin;
            HalfEdge_vertex &point2 = *edge2.origin;

            fdata.push_back(point0.x); fdata.push_back(point0.y); fdata.push_back(point0.z);
            fdata.push_back(point1.x); fdata.push_back(point1.y); fdata.push_back(point1.z);
            fdata.push_back(point2.x); fdata.push_back(point2.y); fdata.push_back(point2.z);
        }

        this->VAO.destroy();
        this->VBO.destroy();
        this->VAO.create(1);
        this->VBO.create(1);

        glBindVertexArray(this->VAO.get());
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO.get());
        glBufferData(GL_ARRAY_BUFFER, fdata.size() * sizeof(GLfloat), fdata.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    virtual void update(double now, double deltaUpdateTime)
    {
        if (this->oldKeyQState == GLFW_PRESS && this->nowKeyQState == GLFW_RELEASE) {
            this->TrySwitchMesh(-1);
        }
        if (this->oldKeyEState == GLFW_PRESS && this->nowKeyEState == GLFW_RELEASE) {
            this->TrySwitchMesh(+1);
        }
    }

    virtual void render(double now, double deltaRenderTime, const glm::mat4 &view, const glm::mat4 &projection) {
        this->guard.ensure();
        this->shader.use();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, this->position);
        this->shader.setMat4("projection", projection);
        this->shader.setMat4("view", view);
        this->shader.setMat4("model", model);

        glBindVertexArray(this->VAO.get());
        this->shader.setVec4("color", this->color);
        glDrawArrays(GL_TRIANGLES, 0, this->meshs[curr].face.size() * 3);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        {
            this->shader.setVec4("color", glm::vec4(1, 1, 1, 1.0f));
            glDrawArrays(GL_TRIANGLES, 0, this->meshs[curr].face.size() * 3);
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

};
