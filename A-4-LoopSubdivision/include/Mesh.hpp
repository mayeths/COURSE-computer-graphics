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
    unsigned id = 0;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    vertex_t *adjust = nullptr; //new position after adjust
    edge_t *asOrigin = nullptr;  //edge which starts from this point
    edge_t *asInsert = nullptr;  //after subdivision
    bool beenOrigined = false;
};

struct face_t {
    edge_t *boundary = nullptr;  //one edge that bound this face
    glm::vec3 normal = glm::vec3(0, 0, 0);
    unsigned id = 0;
};

struct edge_t {
    unsigned id = 0;
    vertex_t *origin = nullptr;
    edge_t *twin = nullptr;
    face_t *incidentFace = nullptr;
    edge_t *nextEdge = nullptr;
    edge_t *preEdge= nullptr;
    vertex_t *insert = nullptr; //record new vertex
    bool isCrease = false; // artificially specify a cease or boundary
    bool CreaseChecked = false;
};

struct mesh_t {
    std::vector<vertex_t> vertex;
    std::vector<edge_t> edge;
    std::vector<face_t> face;
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
    // uint32_t nvertices = 0, nfaces = 0, count_edge_read = 0, count_face_read = 0;
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
        mesh.vertex.resize(nvertices);
        mesh.face.resize(nfaces);
        mesh.edge.resize(nfaces * 3);
        for (size_t i = 0; i < nvertices; i++) {
            std::array<double, 3> positions = this->vertices[i];
            vertex_t &vertex = mesh.vertex[i];
            vertex.x = positions[0];
            vertex.y = positions[1];
            vertex.z = positions[2];
            vertex.id = i;
        }
        for (size_t i = 0; i < nfaces; i++) {
            std::array<int, 3> points = this->faces[i];
            vertex_t &point0 = mesh.vertex[points[0]];
            vertex_t &point1 = mesh.vertex[points[1]];
            vertex_t &point2 = mesh.vertex[points[2]];

            face_t &face = mesh.face[i];
            edge_t &edge0 = mesh.edge[i * 3 + 0];
            edge_t &edge1 = mesh.edge[i * 3 + 1];
            edge_t &edge2 = mesh.edge[i * 3 + 2];

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
            face.id = i;
            edge0.id = i * 3 + 0;
            edge1.id = i * 3 + 1;
            edge2.id = i * 3 + 2;
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
                newMesh.vertex.reserve(oldMesh.vertex.size() * 8);
                newMesh.face.reserve(oldMesh.face.size() * 8);
                newMesh.edge.reserve(oldMesh.edge.size() * 8);
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
            face_t &face = mesh.face[i];
            edge_t &edge0 = *face.boundary;
            edge_t &edge1 = *edge0.nextEdge;
            edge_t &edge2 = *edge0.preEdge;
            vertex_t &point0 = *edge0.origin;
            vertex_t &point1 = *edge1.origin;
            vertex_t &point2 = *edge2.origin;

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


    void loopSubdivision(mesh_t *inputMesh, mesh_t *outputMesh)
    {
        mesh_t &coarse = *inputMesh;
        mesh_t &fine = *outputMesh;
        unsigned vertex_count = 0;
        // old vertex
        for (int i = 0; i < coarse.edge.size(); i++) {
            edge_t &edge = coarse.edge[i];
            if (edge.insert)
                continue;
            fine.vertex.resize(fine.vertex.size() + 1);
            vertex_t &insertV = fine.vertex.back();
            insertV.x = this->loopFomular(&edge, 'x');
            insertV.y = this->loopFomular(&edge, 'y');
            insertV.z = this->loopFomular(&edge, 'z');
            insertV.id = vertex_count;
            vertex_count += 1;
            edge.insert = &insertV;
            if (edge.twin != nullptr)
                edge.twin->insert = &insertV;
        }
        // even vertex
        unsigned vertex_count_old = 0;
        for (int i = 0; i < coarse.vertex.size(); i++) {
            vertex_t &vertex = coarse.vertex[i];
            edge_t *find = vertex.asOrigin;
            std::vector<vertex_t> neighbour;
            unsigned valences = 0;
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
            vertexUpdate.x = this->adjustFomular(&vertexUpdate, valences, 'x', neighbour);
            vertexUpdate.y = this->adjustFomular(&vertexUpdate, valences, 'y', neighbour);
            vertexUpdate.z = this->adjustFomular(&vertexUpdate, valences, 'z', neighbour);
            vertex.id = vertex_count;
            fine.vertex.push_back(vertexUpdate);
            coarse.vertex[i].adjust = &fine.vertex.back();
            vertex_count += 1;
        }
        // remesh
        for (int i = 0; i < coarse.face.size(); i++) {
            face_t &face = coarse.face[i];
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

            for (unsigned i = 0; i < 4; i++) {
                fine.edge.push_back(e[i][0]);
                fine.edge.push_back(e[i][1]);
                fine.edge.push_back(e[i][2]);
                fine.face.resize(fine.face.size() + 1);
                edge_t &e0 = fine.edge[fine.edge.size() - 3];
                edge_t &e1 = fine.edge[fine.edge.size() - 2];
                edge_t &e2 = fine.edge[fine.edge.size() - 1];
                face_t &fine_face = fine.face.back();
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

            e_origin.insert->asOrigin = &fine.edge[fine.edge.size() - 12];
            e_next.insert->asOrigin = &fine.edge[fine.edge.size() - 10];
            e_previous.insert->asOrigin = &fine.edge[fine.edge.size() - 7];
            e_next.origin->adjust->asOrigin = &fine.edge[fine.edge.size() - 11];
            e_previous.origin->adjust->asOrigin = &fine.edge[fine.edge.size() - 8];
            e_origin.origin->adjust->asOrigin = &fine.edge[fine.edge.size() - 5];
        }

        this->findTwin(outputMesh, outputMesh->edge.size());
        this->setNormal(outputMesh);
        this->setCrease(outputMesh);
    }

    double loopFomular(edge_t *insertEdge, char direction)
    {
        if (insertEdge->twin == nullptr || insertEdge->isCrease) {
            vertex_t *a = insertEdge->origin;
            vertex_t *b = insertEdge->nextEdge->origin;
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
            vertex_t *a = insertEdge->origin;
            vertex_t *b = insertEdge->nextEdge->origin;
            vertex_t *c = insertEdge->preEdge->origin;
            vertex_t *d = insertEdge->twin->preEdge->origin;
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

    double adjustFomular(vertex_t *v, unsigned valence, char direction, std::vector<vertex_t> neighbour)
    {
        edge_t *edge = v->asOrigin;
        if (edge->twin == nullptr || edge->isCrease) {
            vertex_t *a = edge->insert;
            vertex_t *b = edge->preEdge->insert;
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


    //find the twin of edges
    void findTwin(mesh_t *mesh, unsigned edge_count)
    {
        for (unsigned i = 0; i < edge_count; i++)
            for (unsigned j = i + 1; j < edge_count; j++) {
                if ((((*mesh).edge)[i]).twin)
                    break;
                if ((((*mesh).edge)[j]).twin)
                    continue;
                if ((((*mesh).edge)[i]).origin == (*(((*mesh).edge)[j]).nextEdge).origin && (((*mesh).edge)[j]).origin == (*(((*mesh).edge)[i]).nextEdge).origin) {
                    (((*mesh).edge)[i]).twin = &((*mesh).edge)[j];
                    (((*mesh).edge)[j]).twin = &((*mesh).edge)[i];
                }
            }
    }
    //compute normal
    void setNormal(mesh_t *mesh)
    {
        unsigned i = 0;
        log_trace("total face %d", (*mesh).face.size());
        for (auto it = (*mesh).face.begin(); it != (*mesh).face.end(); it++, i++) {
            log_trace("face %p(%u)", it, (*it).id);
            edge_t *origin = (*it).boundary;
            log_trace("    boundary %p(%u)", origin, (*origin).id);
            // v1 is first vector of the face, v2 is the second vector, vn is the normal
            glm::vec3 v1, v2, vn;
            vertex_t &p0 = *(*origin).origin;
            log_trace("    p0 %p(%u)", &p0, p0.id);
            vertex_t &p1 = *(*(*origin).nextEdge).origin;
            log_trace("    p1 %p(%u)", &p1, p1.id);
            vertex_t &p2 = *(*(*origin).preEdge).origin;
            log_trace("    p2 %p(%u)", &p2, p2.id);
            double length;
            //compute normal
            v1.x = p0.x - p1.x;
            v1.y = p0.y - p1.y;
            v1.z = p0.z - p1.z;
            v2.x = p0.x - p2.x;
            v2.y = p0.y - p2.y;
            v2.z = p0.z - p2.z;
            vn.x = v1.y * v2.z - v1.z * v2.y;
            vn.y = v1.z * v2.x - v1.x * v2.z;
            vn.z = v1.x * v2.y - v1.y * v2.x;
            length = sqrt((vn.x * vn.x) + (vn.y * vn.y) + (vn.z * vn.z));
            if (length == 0.0) {
                std::cout << "Wrong id, a vector's length is 0.";
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
    //set crease
    void setCrease(mesh_t *mesh)
    {
        for (auto it = (*mesh).edge.begin(); it != (*mesh).edge.end(); it++) {
            if ((*it).CreaseChecked)
                continue;
            // log_trace("edge %p(%u)", it, (*it).id);
            glm::vec3 &normal0 = (*(*it).incidentFace).normal;
            // log_trace("    incidentFace %p(%u)", (*(*it).incidentFace), (*(*it).incidentFace).id);
            glm::vec3 &normal1 = (*(*(*it).twin).incidentFace).normal;
            // log_trace("    twinFace %p(%u)", (*(*(*it).twin).incidentFace), (*(*(*it).twin).incidentFace).id);
            //the function acos return Arc system
            double angle = acos(vectorAngle(normal0, normal1)) * 180.0 / M_PI;
            /*how to know it is a crease*/
            if (angle >= 150.0) {
                (*it).isCrease = true;
                (*(*it).twin).isCrease = true;
            }
            (*it).CreaseChecked = true;
            (*(*it).twin).CreaseChecked = true;
        }
    }


};
