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
                this->loopSubdivision(&oldMesh, &newMesh);
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


    void loopSubdivision(HalfEdge_mesh *inputMesh, HalfEdge_mesh *outputMesh)
    {
        HalfEdge_mesh &coarse = *inputMesh;
        HalfEdge_mesh &fine = *outputMesh;
        unsigned vertex_count = 0;
        // old vertex
        for (int i = 0; i < coarse.halfedge.size(); i++) {
            HalfEdge_halfedge &edge = coarse.halfedge[i];
            if (edge.insert)
                continue;
            fine.vertex.resize(fine.vertex.size() + 1);
            HalfEdge_vertex &insertV = fine.vertex.back();
            insertV.x = this->loopFomular(&edge, 'x');
            insertV.y = this->loopFomular(&edge, 'y');
            insertV.z = this->loopFomular(&edge, 'z');
            insertV.number = vertex_count;
            vertex_count += 1;
            edge.insert = &insertV;
            if (edge.twin != nullptr)
                edge.twin->insert = &insertV;
        }
        // even vertex
        unsigned vertex_count_old = 0;
        for (int i = 0; i < coarse.vertex.size(); i++) {
            HalfEdge_vertex &vertex = coarse.vertex[i];
            HalfEdge_halfedge *find = vertex.asOrigin;
            std::vector<HalfEdge_vertex> neighbour;
            unsigned valences = 0;
            do {
                valences += 1;
                neighbour.push_back(*find->insert);
                if (find->twin == nullptr) {
                    HalfEdge_halfedge *findBack = vertex.asOrigin->preEdge;
                    while (findBack != nullptr) {
                        valences += 1;
                        neighbour.push_back(*findBack->insert);
                        if (findBack->twin == nullptr)
                            break;
                        findBack = findBack->twin->preEdge;
                    }
                    break;
                }
                find = find->twin->nextEdge;
            } while (find != vertex.asOrigin);

            HalfEdge_vertex vertexUpdate;
            vertexUpdate = vertex;
            vertexUpdate.x = this->adjustFomular(&vertexUpdate, valences, 'x', neighbour);
            vertexUpdate.y = this->adjustFomular(&vertexUpdate, valences, 'y', neighbour);
            vertexUpdate.z = this->adjustFomular(&vertexUpdate, valences, 'z', neighbour);
            vertex.number = vertex_count;
            fine.vertex.push_back(vertexUpdate);
            coarse.vertex[i].adjust = &fine.vertex.back();
            vertex_count += 1;
        }
        // remesh
        for (int i = 0; i < coarse.face.size(); i++) {
            HalfEdge_face &face = coarse.face[i];
            HalfEdge_halfedge e[4][3];
            HalfEdge_halfedge &e_origin = *face.boundary;
            HalfEdge_halfedge &e_previous = *e_origin.preEdge;
            HalfEdge_halfedge &e_next = *e_origin.nextEdge;

            e[0][0].origin = e_origin.insert;
            e[0][1].origin = (e_next.origin)->adjust;
            e[0][2].origin = e_next.insert;

            e[1][0].origin = e_next.insert;
            e[1][1].origin = (e_previous.origin)->adjust;
            e[1][2].origin = e_previous.insert;

            e[2][0].origin = e_previous.insert;
            e[2][1].origin = (e_origin.origin)->adjust;
            e[2][2].origin = e_origin.insert;

            e[3][0].origin = e_origin.insert;
            e[3][1].origin = e_next.insert;
            e[3][2].origin = e_previous.insert;

            for (unsigned i = 0; i < 4; i++) {
                fine.halfedge.push_back(e[i][0]);
                fine.halfedge.push_back(e[i][1]);
                fine.halfedge.push_back(e[i][2]);
                fine.face.resize(fine.face.size() + 1);
                HalfEdge_halfedge &e0 = fine.halfedge[fine.halfedge.size() - 3];
                HalfEdge_halfedge &e1 = fine.halfedge[fine.halfedge.size() - 2];
                HalfEdge_halfedge &e2 = fine.halfedge[fine.halfedge.size() - 1];
                HalfEdge_face &fine_face = fine.face.back();
                e0.incidentFace = &fine_face;
                e1.incidentFace = &fine_face;
                e2.incidentFace = &fine_face;
                e0.nextEdge = &e1;
                e1.nextEdge = &e2;
                e2.nextEdge = &e0;
                e0.preEdge = &e2;
                e1.preEdge = &e0;
                e2.preEdge = &e1;
                fine_face.boundary = &e0;
            }

            e_origin.insert->asOrigin = &fine.halfedge[fine.halfedge.size() - 12];
            e_next.origin->adjust->asOrigin = &fine.halfedge[fine.halfedge.size() - 11];
            e_next.insert->asOrigin = &fine.halfedge[fine.halfedge.size() - 10];
            e_previous.origin->adjust->asOrigin = &fine.halfedge[fine.halfedge.size() - 8];
            e_previous.insert->asOrigin = &fine.halfedge[fine.halfedge.size() - 7];
            e_origin.origin->adjust->asOrigin = &fine.halfedge[fine.halfedge.size() - 5];
        }

        findTwin(outputMesh, outputMesh->halfedge.size());
        setNormal(outputMesh);
        setCrease(outputMesh);
    }

    double loopFomular(HalfEdge_halfedge *insertHalfedge, char direction)
    {
        if (insertHalfedge->twin == nullptr || insertHalfedge->isCrease) {
            HalfEdge_vertex *a = insertHalfedge->origin;
            HalfEdge_vertex *b = insertHalfedge->nextEdge->origin;
            if (direction == 'x') {
                return (a->x + b->x) / 2.0;
            } else if (direction == 'y') {
                return (a->y + b->y) / 2.0;
            } else if (direction == 'z') {
                return (a->z + b->z) / 2.0;
            } else {
                throw std::runtime_error("you should not come here 1.0");
            }
        } else {
            HalfEdge_vertex *a = insertHalfedge->origin;
            HalfEdge_vertex *b = insertHalfedge->nextEdge->origin;
            HalfEdge_vertex *c = insertHalfedge->preEdge->origin;
            HalfEdge_vertex *d = insertHalfedge->twin->preEdge->origin;
            if (direction == 'x') {
                return (3.0 / 8.0) * (a->x + b->x) + (1.0 / 8.0) * (c->x + d->x);
            } else if (direction == 'y') {
                return (3.0 / 8.0) * (a->y + b->y) + (1.0 / 8.0) * (c->y + d->y);
            } else if (direction == 'z') {
                return (3.0 / 8.0) * (a->z + b->z) + (1.0 / 8.0) * (c->z + d->z);
            } else {
                throw std::runtime_error("you should not come here 2.0");
            }
        }
    }

    double adjustFomular(HalfEdge_vertex *v, unsigned valence, char direction, vector<HalfEdge_vertex> neighbour)
    {
        HalfEdge_halfedge *edge = v->asOrigin;
        if (edge->twin == nullptr || edge->isCrease) {
            HalfEdge_vertex *a = edge->insert;
            HalfEdge_vertex *b = edge->preEdge->insert;
            if (direction == 'x')
                return (1.0 / 8.0) * (a->x + b->x) + (3.0 / 4.0) * (v->x);
            else if (direction == 'y')
                return (1.0 / 8.0) * (a->y + b->y) + (3.0 / 4.0) * (v->y);
            else if (direction == 'z')
                return (1.0 / 8.0) * (a->z + b->z) + (3.0 / 4.0) * (v->z);
            else
                throw std::runtime_error("you should not come here 3.0");
        } else {
            double n = (double)valence;
            double beta = 0;

            /* Loop's suggestion for beta */
            double x = (3.0 / 8.0 + 1.0 / 4.0 * cos(2.0 * Pi / n));
            beta = (1.0 / n) * (5.0 / 8.0 - x * x);
            /* Warren's suggestion for beta */
            // if (valence == 3)
            //     beta = 3.0 / 16.0;
            // else if (valence > 3)
            //     beta = 3.0 / (8.0 * n);
            // else
            //     throw std::runtime_error("you should not come here 4.0.0");

            double sum = 0;
            if (direction == 'x') {
                for (int i = 0; i < neighbour.size(); i++)
                    sum += neighbour[i].x;
                return v->x * (1.0 - n * beta) + sum * beta;
            } else if (direction == 'y') {
                for (int i = 0; i < neighbour.size(); i++)
                    sum += neighbour[i].y;
                return v->y * (1.0 - n * beta) + sum * beta;
            } else if (direction == 'z') {
                for (int i = 0; i < neighbour.size(); i++)
                    sum += neighbour[i].z;
                return v->z * (1.0 - n * beta) + sum * beta;
            } else {
                throw std::runtime_error("you should not come here 4.0");
            }

        }
    }

};
