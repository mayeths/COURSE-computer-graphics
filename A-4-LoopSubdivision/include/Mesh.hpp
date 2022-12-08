#pragma once

#include <stddef.h>
#include <array>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "raii.hpp"
#include "log.h"
#include "Shader.hpp"
#include "Object/DrawableObject.hpp"

struct vertex_t;
struct edge_t;
struct face_t;

struct vertex_t {
    uint32_t ID = 0;
    glm::vec3 position = glm::vec3(0, 0, 0);
    vertex_t *adjust = nullptr; //new position after adjust
    edge_t *asOrigin = nullptr;  //edge which starts from this point
    edge_t *asInsert = nullptr;  //after subdivision
};

struct face_t {
    uint32_t ID = 0;
    edge_t *boundary = nullptr;  //one edge that bound this face
    glm::vec3 normal = glm::vec3(0, 0, 0);
};

struct edge_t {
    uint32_t ID = 0;
    vertex_t *origin = nullptr;
    edge_t *twin = nullptr;
    face_t *incidentFace = nullptr;
    edge_t *nextEdge = nullptr;
    edge_t *preEdge= nullptr;
    vertex_t *insert = nullptr; //record new vertex
    bool isCrease = false; // artificially specify a cease or boundary
};

struct mesh_t {
    uint32_t uuid_v;
    uint32_t uuid_e;
    uint32_t uuid_f;
    std::vector<vertex_t> vertices;
    std::vector<edge_t> edges;
    std::vector<face_t> faces;
};

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
    std::vector<mesh_t> meshs;
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
        mesh_t &mesh = this->meshs[0];

        size_t nvertices = this->vertices.size();
        size_t nfaces = this->faces.size();
        mesh.vertices.resize(nvertices);
        mesh.faces.resize(nfaces);
        mesh.edges.resize(nfaces * 3);
        for (size_t i = 0; i < nvertices; i++) {
            std::array<double, 3> positions = this->vertices[i];
            vertex_t &vertex = mesh.vertices[i];
            vertex.position.x = positions[0];
            vertex.position.y = positions[1];
            vertex.position.z = positions[2];
            vertex.ID = mesh.uuid_v++;
        }

        std::map<const vertex_t *, bool> associated;
        for (size_t i = 0; i < nfaces; i++) {
            std::array<int, 3> points = this->faces[i];
            vertex_t &point0 = mesh.vertices[points[0]];
            vertex_t &point1 = mesh.vertices[points[1]];
            vertex_t &point2 = mesh.vertices[points[2]];

            face_t &face = mesh.faces[i];
            edge_t &edge0 = mesh.edges[i * 3 + 0];
            edge_t &edge1 = mesh.edges[i * 3 + 1];
            edge_t &edge2 = mesh.edges[i * 3 + 2];

            face.boundary = &edge0;
            edge0.origin = &point0;
            edge1.origin = &point1;
            edge2.origin = &point2;
            if (!associated[&point0]) {
                point0.asOrigin = &edge0;
                associated[&point0] = true;
            }
            if (!associated[&point1]) {
                point1.asOrigin = &edge1;
                associated[&point1] = true;
            }
            if (!associated[&point2]) {
                point2.asOrigin = &edge2;
                associated[&point2] = true;
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
            face.ID = mesh.uuid_f++;
            edge0.ID = mesh.uuid_e++;
            edge1.ID = mesh.uuid_e++;
            edge2.ID = mesh.uuid_e++;
        }

        this->setNormal(&mesh);
        this->findTwin(&mesh, nfaces * 3);
        this->setCrease(&mesh);

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
                mesh_t &oldMesh = this->meshs[i-1];
                mesh_t &newMesh = this->meshs[i];
                newMesh.vertices.reserve(oldMesh.vertices.size() * 8);
                newMesh.faces.reserve(oldMesh.faces.size() * 8);
                newMesh.edges.reserve(oldMesh.edges.size() * 8);
                this->loopSubdivision(&oldMesh, &newMesh);
            }
        }
        this->curr = targetIndex;
        this->OffloadCurrentMesh();
    }

    void OffloadCurrentMesh()
    {
        mesh_t &mesh = this->meshs[this->curr];
        size_t nfaces = this->faces.size();
        std::vector<GLfloat> fdata;
        for (size_t i = 0; i < nfaces; i++) {
            face_t &face = mesh.faces[i];
            edge_t &edge0 = *face.boundary;
            edge_t &edge1 = *edge0.nextEdge;
            edge_t &edge2 = *edge0.preEdge;
            vertex_t &point0 = *edge0.origin;
            vertex_t &point1 = *edge1.origin;
            vertex_t &point2 = *edge2.origin;

            fdata.push_back(point0.position.x); fdata.push_back(point0.position.y); fdata.push_back(point0.position.z);
            fdata.push_back(point1.position.x); fdata.push_back(point1.position.y); fdata.push_back(point1.position.z);
            fdata.push_back(point2.position.x); fdata.push_back(point2.position.y); fdata.push_back(point2.position.z);
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
        glDrawArrays(GL_TRIANGLES, 0, this->meshs[curr].faces.size() * 3);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        {
            this->shader.setVec4("color", glm::vec4(1, 1, 1, 1.0f));
            glDrawArrays(GL_TRIANGLES, 0, this->meshs[curr].faces.size() * 3);
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }


    void loopSubdivision(mesh_t *inputMesh, mesh_t *outputMesh)
    {
        mesh_t &coarse = *inputMesh;
        mesh_t &fine = *outputMesh;
        uint32_t vertex_count = 0;
        // old vertex
        for (int i = 0; i < coarse.edges.size(); i++) {
            edge_t &edge = coarse.edges[i];
            if (edge.insert)
                continue;
            fine.vertices.resize(fine.vertices.size() + 1);
            vertex_t &insertV = fine.vertices.back();
            insertV.position.x = this->loopFomular(&edge, 'x');
            insertV.position.y = this->loopFomular(&edge, 'y');
            insertV.position.z = this->loopFomular(&edge, 'z');
            insertV.ID = fine.uuid_v++;
            vertex_count += 1;
            edge.insert = &insertV;
            if (edge.twin != nullptr)
                edge.twin->insert = &insertV;
        }
        // even vertex
        uint32_t vertex_count_old = 0;
        for (int i = 0; i < coarse.vertices.size(); i++) {
            vertex_t &vertex = coarse.vertices[i];
            edge_t *find = vertex.asOrigin;
            std::vector<vertex_t> neighbour;
            uint32_t valences = 0;
            do {
                valences += 1;
                neighbour.push_back(*find->insert);
                if (find->twin == nullptr) {
                    edge_t *findBack = vertex.asOrigin->preEdge;
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

            vertex_t vertexUpdate;
            vertexUpdate = vertex;
            vertexUpdate.position.x = this->adjustFomular(&vertexUpdate, valences, 'x', neighbour);
            vertexUpdate.position.y = this->adjustFomular(&vertexUpdate, valences, 'y', neighbour);
            vertexUpdate.position.z = this->adjustFomular(&vertexUpdate, valences, 'z', neighbour);
            vertexUpdate.ID = fine.uuid_v++;
            fine.vertices.push_back(vertexUpdate);
            coarse.vertices[i].adjust = &fine.vertices.back();
            vertex_count += 1;
        }
        // remesh
        for (int i = 0; i < coarse.faces.size(); i++) {
            face_t &face = coarse.faces[i];
            edge_t e[4][3];
            edge_t &e_origin = *face.boundary;
            edge_t &e_previous = *e_origin.preEdge;
            edge_t &e_next = *e_origin.nextEdge;

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

            for (uint32_t i = 0; i < 4; i++) {
                fine.edges.push_back(e[i][0]);
                fine.edges.push_back(e[i][1]);
                fine.edges.push_back(e[i][2]);
                fine.faces.resize(fine.faces.size() + 1);
                edge_t &e0 = fine.edges[fine.edges.size() - 3];
                edge_t &e1 = fine.edges[fine.edges.size() - 2];
                edge_t &e2 = fine.edges[fine.edges.size() - 1];
                face_t &fine_face = fine.faces.back();
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

            e_origin.insert->asOrigin = &fine.edges[fine.edges.size() - 12];
            e_next.insert->asOrigin = &fine.edges[fine.edges.size() - 10];
            e_previous.insert->asOrigin = &fine.edges[fine.edges.size() - 7];
            e_next.origin->adjust->asOrigin = &fine.edges[fine.edges.size() - 11];
            e_previous.origin->adjust->asOrigin = &fine.edges[fine.edges.size() - 8];
            e_origin.origin->adjust->asOrigin = &fine.edges[fine.edges.size() - 5];
        }

        this->findTwin(outputMesh, outputMesh->edges.size());
        this->setNormal(outputMesh);
        this->setCrease(outputMesh);
    }

    double loopFomular(edge_t *insertEdge, char direction)
    {
        if (insertEdge->twin == nullptr || insertEdge->isCrease) {
            vertex_t *a = insertEdge->origin;
            vertex_t *b = insertEdge->nextEdge->origin;
            if (direction == 'x') {
                return (a->position.x + b->position.x) / 2.0;
            } else if (direction == 'y') {
                return (a->position.y + b->position.y) / 2.0;
            } else if (direction == 'z') {
                return (a->position.z + b->position.z) / 2.0;
            } else {
                throw std::runtime_error("you should not come here 1.0");
            }
        } else {
            vertex_t *a = insertEdge->origin;
            vertex_t *b = insertEdge->nextEdge->origin;
            vertex_t *c = insertEdge->preEdge->origin;
            vertex_t *d = insertEdge->twin->preEdge->origin;
            if (direction == 'x') {
                return (3.0 / 8.0) * (a->position.x + b->position.x) + (1.0 / 8.0) * (c->position.x + d->position.x);
            } else if (direction == 'y') {
                return (3.0 / 8.0) * (a->position.y + b->position.y) + (1.0 / 8.0) * (c->position.y + d->position.y);
            } else if (direction == 'z') {
                return (3.0 / 8.0) * (a->position.z + b->position.z) + (1.0 / 8.0) * (c->position.z + d->position.z);
            } else {
                throw std::runtime_error("you should not come here 2.0");
            }
        }
    }

    double adjustFomular(vertex_t *v, uint32_t valence, char direction, std::vector<vertex_t> neighbour)
    {
        edge_t *edge = v->asOrigin;
        if (edge->twin == nullptr || edge->isCrease) {
            vertex_t *a = edge->insert;
            vertex_t *b = edge->preEdge->insert;
            if (direction == 'x')
                return (1.0 / 8.0) * (a->position.x + b->position.x) + (3.0 / 4.0) * (v->position.x);
            else if (direction == 'y')
                return (1.0 / 8.0) * (a->position.y + b->position.y) + (3.0 / 4.0) * (v->position.y);
            else if (direction == 'z')
                return (1.0 / 8.0) * (a->position.z + b->position.z) + (3.0 / 4.0) * (v->position.z);
            else
                throw std::runtime_error("you should not come here 3.0");
        } else {
            double n = (double)valence;
            double beta = 0;

            /* Loop's suggestion for beta */
            double x = (3.0 / 8.0 + 1.0 / 4.0 * cos(2.0 * M_PI / n));
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
                    sum += neighbour[i].position.x;
                return v->position.x * (1.0 - n * beta) + sum * beta;
            } else if (direction == 'y') {
                for (int i = 0; i < neighbour.size(); i++)
                    sum += neighbour[i].position.y;
                return v->position.y * (1.0 - n * beta) + sum * beta;
            } else if (direction == 'z') {
                for (int i = 0; i < neighbour.size(); i++)
                    sum += neighbour[i].position.z;
                return v->position.z * (1.0 - n * beta) + sum * beta;
            } else {
                throw std::runtime_error("you should not come here 4.0");
            }

        }
    }


    //find the twin of edges
    void findTwin(mesh_t *mesh, uint32_t edge_count)
    {
        for (uint32_t i = 0; i < edge_count; i++)
            for (uint32_t j = i + 1; j < edge_count; j++) {
                if ((((*mesh).edges)[i]).twin)
                    break;
                if ((((*mesh).edges)[j]).twin)
                    continue;
                if ((((*mesh).edges)[i]).origin == (*(((*mesh).edges)[j]).nextEdge).origin && (((*mesh).edges)[j]).origin == (*(((*mesh).edges)[i]).nextEdge).origin) {
                    (((*mesh).edges)[i]).twin = &((*mesh).edges)[j];
                    (((*mesh).edges)[j]).twin = &((*mesh).edges)[i];
                }
            }
    }
    //compute normal
    void setNormal(mesh_t *mesh)
    {
        uint32_t i = 0;
        // log_trace("total face %d", (*mesh).faces.size());
        for (auto it = (*mesh).faces.begin(); it != (*mesh).faces.end(); it++, i++) {
            // log_trace("face %p(%u)", it, (*it).ID);
            edge_t *origin = (*it).boundary;
            // log_trace("    boundary %p(%u)", origin, (*origin).ID);
            // v1 is first vector of the face, v2 is the second vector, vn is the normal
            glm::vec3 v1, v2, vn;
            vertex_t &p0 = *(*origin).origin;
            // log_trace("    p0 %p(%u)", &p0, p0.ID);
            vertex_t &p1 = *(*(*origin).nextEdge).origin;
            // log_trace("    p1 %p(%u)", &p1, p1.ID);
            vertex_t &p2 = *(*(*origin).preEdge).origin;
            // log_trace("    p2 %p(%u)", &p2, p2.ID);
            double length;
            //compute normal
            v1.x = p0.position.x - p1.position.x;
            v1.y = p0.position.y - p1.position.y;
            v1.z = p0.position.z - p1.position.z;
            v2.x = p0.position.x - p2.position.x;
            v2.y = p0.position.y - p2.position.y;
            v2.z = p0.position.z - p2.position.z;
            vn.x = v1.y * v2.z - v1.z * v2.y;
            vn.y = v1.z * v2.x - v1.x * v2.z;
            vn.z = v1.x * v2.y - v1.y * v2.x;
            length = sqrt((vn.x * vn.x) + (vn.y * vn.y) + (vn.z * vn.z));
            if (length == 0.0) {
                std::cout << "Wrong ID, a vector's length is 0.";
                return;
            }
            vn.x /= length;
            vn.y /= length;
            vn.z /= length;
            (*it).normal = vn;
        }
    }
    //compute cos(angle) of two vectors
    double vectorAngle(const glm::vec3 &v0, const glm::vec3 &v1)
    {
        double length1 = sqrt((v0.x * v0.x) + (v0.y * v0.y) + (v0.z * v0.z));
        double length2 = sqrt((v1.x * v1.x) + (v1.y * v1.y) + (v1.z * v1.z));
        return ((v0.x * v1.x + v0.y * v1.y + v0.z * v1.z) / (length1 * length2));
    }

    void setCrease(mesh_t *mesh)
    {
        std::map<const edge_t *, bool> checked;
        for (int i= 0; i < mesh->edges.size(); i++) {
            edge_t *edge = &mesh->edges[i];
            if (checked[edge])
                continue;

            glm::vec3 normal0 = glm::normalize(edge->incidentFace->normal);
            glm::vec3 normal1 = glm::normalize(edge->twin->incidentFace->normal);
            double degrees = glm::degrees(glm::acos(glm::dot(normal0, normal1)));

            if (degrees >= 150) {
                edge->isCrease = true;
                edge->twin->isCrease = true;
            } else {
                edge->isCrease = false;
                edge->twin->isCrease = false;
            }
            checked[edge] = true;
            checked[edge->twin] = true;
        }
    }

};
