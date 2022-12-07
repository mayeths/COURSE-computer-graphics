#pragma once

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
    VAO_raii lineVAO;
    VBO_raii lineVBO;
    std::string meshPath;
    std::vector<std::array<double, 3>> vertices;
    std::vector<std::array<int, 3>> faces;
    uint32_t nvertices = 0, nfaces = 0, count_halfedge_read = 0, count_face_read = 0;
    HalfEdge_mesh mesh;
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
        // this->mesh.vertex.reserve(100);
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

                // std::array<int, 3> edge;
                // instring >> edge[0] >> edge[1] >> edge[2];
                // this->faces.push_back(std::move(edge));
                // count_halfedge_read += 3;
                // HalfEdge_halfedge edge_temp1;
                // HalfEdge_halfedge edge_temp2;
                // HalfEdge_halfedge edge_temp3;
                // HalfEdge_face face_temp;
                // this->mesh.halfedge.push_back(edge_temp1);
                // this->mesh.halfedge.push_back(edge_temp2);
                // this->mesh.halfedge.push_back(edge_temp3);
                // log_trace("    halfedge1 %p", &mesh.halfedge[count_halfedge_read - 3]);
                // log_trace("    halfedge2 %p", &mesh.halfedge[count_halfedge_read - 2]);
                // log_trace("    halfedge3 %p", &mesh.halfedge[count_halfedge_read - 1]);
                // this->mesh.face.push_back(face_temp);
                // this->mesh.face[count_face_read].boundary = &mesh.halfedge[count_halfedge_read - 3];
                // log_trace("    face %u boundary %p(%u)", count_face_read, &mesh.halfedge[count_halfedge_read - 3], mesh.halfedge[count_halfedge_read - 3].number);
				// //halfedge's origin & vertexs
                // this->mesh.halfedge[count_halfedge_read - 3].origin = &mesh.vertex[edge[0]];
                // assert(edge[0] < mesh.vertex.size());
                // log_trace("    face %u origin %p", count_face_read, &mesh.vertex[edge[0]]);
                // if (!mesh.vertex[edge[0]].beenOrigined) {
                //     mesh.vertex[edge[0]].asOrigin = &mesh.halfedge[count_halfedge_read - 3];
                //     mesh.vertex[edge[0]].beenOrigined = true;
                // }
                // (mesh.halfedge[count_halfedge_read - 2]).origin = &mesh.vertex[edge[1]];
                // if (!mesh.vertex[edge[1]].beenOrigined) {
                //     mesh.vertex[edge[1]].asOrigin = &mesh.halfedge[count_halfedge_read - 2];
                //     mesh.vertex[edge[1]].beenOrigined = true;
                // }
                // mesh.halfedge[count_halfedge_read - 1].origin = &mesh.vertex[edge[2]];
                // if (!mesh.vertex[edge[2]].beenOrigined){
                //     mesh.vertex[edge[2]].asOrigin = &mesh.halfedge[count_halfedge_read - 1];
                //     mesh.vertex[edge[2]].beenOrigined = true;
                // }
                // log_trace("    halfedge1 %p", &mesh.halfedge[count_halfedge_read - 3]);
                // log_trace("    halfedge2 %p", &mesh.halfedge[count_halfedge_read - 2]);
                // log_trace("    halfedge3 %p", &mesh.halfedge[count_halfedge_read - 1]);
                // //halfedge's nextedge
                // mesh.halfedge[count_halfedge_read - 3].nextEdge = &mesh.halfedge[count_halfedge_read - 2];
                // mesh.halfedge[count_halfedge_read - 2].nextEdge = &mesh.halfedge[count_halfedge_read - 1];
                // mesh.halfedge[count_halfedge_read - 1].nextEdge = &mesh.halfedge[count_halfedge_read - 3];
                // //halfedge's preedge
                // mesh.halfedge[count_halfedge_read - 3].preEdge = &mesh.halfedge[count_halfedge_read - 1];
                // mesh.halfedge[count_halfedge_read - 2].preEdge = &mesh.halfedge[count_halfedge_read - 3];
                // mesh.halfedge[count_halfedge_read - 1].preEdge = &mesh.halfedge[count_halfedge_read - 2];
                // //halfedge's incidentface
                // mesh.halfedge[count_halfedge_read - 3].incidentFace = &mesh.face[count_face_read];
                // mesh.halfedge[count_halfedge_read - 2].incidentFace = &mesh.face[count_face_read];
                // mesh.halfedge[count_halfedge_read - 1].incidentFace = &mesh.face[count_face_read];
                // //halfedge's name
                // mesh.halfedge[count_halfedge_read - 3].number = count_halfedge_read - 3;
                // mesh.halfedge[count_halfedge_read - 2].number = count_halfedge_read - 2;
                // mesh.halfedge[count_halfedge_read - 1].number = count_halfedge_read - 1;
                // count_face_read++;
            }
        }

        size_t nvertices = this->vertices.size();
        size_t nfaces = this->faces.size();
        this->mesh.vertex.resize(nvertices);
        this->mesh.face.resize(nfaces);
        this->mesh.halfedge.resize(nfaces * 3);
        for (size_t i = 0; i < nvertices; i++) {
            std::array<double, 3> positions = this->vertices[i];
            HalfEdge_vertex &vertex = this->mesh.vertex[i];
            vertex.x = positions[0];
            vertex.y = positions[1];
            vertex.z = positions[2];
            vertex.number = i;
        }
        for (size_t i = 0; i < nfaces; i++) {
            std::array<int, 3> points = this->faces[i];
            HalfEdge_vertex &point0 = this->mesh.vertex[points[0]];
            HalfEdge_vertex &point1 = this->mesh.vertex[points[1]];
            HalfEdge_vertex &point2 = this->mesh.vertex[points[2]];

            HalfEdge_face &face = this->mesh.face[i];
            HalfEdge_halfedge &edge0 = this->mesh.halfedge[i * 3 + 0];
            HalfEdge_halfedge &edge1 = this->mesh.halfedge[i * 3 + 1];
            HalfEdge_halfedge &edge2 = this->mesh.halfedge[i * 3 + 2];

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

        setNormal(&this->mesh);
        findTwin(&this->mesh, nfaces * 3);
        setCrease(&this->mesh);

        log_debug("Mesh %d vertices %d faces", this->vertices.size(), this->faces.size());

        std::vector<GLfloat> fdata;
        std::vector<GLfloat> edata;
        for (size_t i = 0; i < nfaces; i++) {
            HalfEdge_face &face = this->mesh.face[i];
            HalfEdge_halfedge &edge0 = *face.boundary;
            HalfEdge_halfedge &edge1 = *edge0.nextEdge;
            HalfEdge_halfedge &edge2 = *edge0.preEdge;
            HalfEdge_vertex &point0 = *edge0.origin;
            HalfEdge_vertex &point1 = *edge1.origin;
            HalfEdge_vertex &point2 = *edge2.origin;

            fdata.push_back(point0.x); fdata.push_back(point0.y); fdata.push_back(point0.z);
            fdata.push_back(point1.x); fdata.push_back(point1.y); fdata.push_back(point1.z);
            fdata.push_back(point2.x); fdata.push_back(point2.y); fdata.push_back(point2.z);
            edata.push_back(point0.x); edata.push_back(point0.y); edata.push_back(point0.z);
            edata.push_back(point1.x); edata.push_back(point1.y); edata.push_back(point1.z);
            edata.push_back(point1.x); edata.push_back(point1.y); edata.push_back(point1.z);
            edata.push_back(point2.x); edata.push_back(point2.y); edata.push_back(point2.z);
            edata.push_back(point0.x); edata.push_back(point0.y); edata.push_back(point0.z);
            edata.push_back(point2.x); edata.push_back(point2.y); edata.push_back(point2.z);
        }

        this->VAO.create(1);
        this->VBO.create(1);
        this->lineVAO.create(1);
        this->lineVBO.create(1);

        glBindVertexArray(this->VAO.get());
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO.get());
        glBufferData(GL_ARRAY_BUFFER, fdata.size() * sizeof(GLfloat), fdata.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(this->lineVAO.get());
        glBindBuffer(GL_ARRAY_BUFFER, this->lineVBO.get());
        glBufferData(GL_ARRAY_BUFFER, edata.size() * sizeof(GLfloat), edata.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        this->guard.set();
    }

    virtual void update(double now, double deltaUpdateTime) {}

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
        glDrawArrays(GL_TRIANGLES, 0, this->mesh.face.size() * 3);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        {
            this->shader.setVec4("color", glm::vec4(1, 1, 1, 1.0f));
            glDrawArrays(GL_TRIANGLES, 0, this->mesh.face.size() * 3);
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // glBindVertexArray(this->lineVAO.get());
        // this->shader.setVec4("color", glm::vec4(1, 1, 1, 1.0f));
        // glDrawArrays(GL_LINES, 0, this->mesh.face.size() * 6);
        // glBindVertexArray(0);
    }

};
