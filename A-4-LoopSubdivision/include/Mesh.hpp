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

#include "util/raii.hpp"
#include "util/log.h"
#include "framework/Shader.hpp"
#include "framework/Object.hpp"
#include "framework/GUI.hpp"

struct vertex_t;
struct edge_t;
struct face_t;

struct vertex_t {
    uint32_t ID = 0;
    glm::vec3 position = glm::vec3(0, 0, 0);
    edge_t *belong = nullptr;  //edge which starts from this vertex
};

struct edge_t {
    uint32_t ID = 0;
    vertex_t *vertex = nullptr;
    face_t *belong = nullptr;
    edge_t *twin = nullptr;
    edge_t *next = nullptr;
    edge_t *prev = nullptr;
    bool crease = false; // A crease (or boundary)
};

struct face_t {
    uint32_t ID = 0;
    edge_t *edge = nullptr;
    glm::vec3 normal = glm::vec3(0, 0, 0);
};

struct mesh_t {
    uint32_t uuid_v;
    uint32_t uuid_e;
    uint32_t uuid_f;
    std::vector<vertex_t> vertices;
    std::vector<edge_t> edges;
    std::vector<face_t> faces;
};

class Mesh : public Object, public GUIHandler
{
    guard_t guard;
public:
    Shader shader;
    VAO_raii VAO;
    VBO_raii VBO;
    std::string meshPath;
    std::vector<mesh_t> meshs;
    int curr = 0;
    int oldKeyQState = GLFW_RELEASE;
    int oldKeyEState = GLFW_RELEASE;
    int nowKeyQState = GLFW_RELEASE;
    int nowKeyEState = GLFW_RELEASE;
    glm::vec4 color = glm::vec4(0, 0, 0, 1.0);
    double boundingBoxSize = 10;

    void SetShaderPath(const std::string vertexPath, const std::string fragmentPath)
    {
        this->shader.vertexPath = vertexPath;
        this->shader.fragmentPath = fragmentPath;
    }
    void SetMeshPath(const std::string &meshPath) {
        this->meshPath = meshPath;
    }
    void SetBoundingBoxSize(double boundingBoxSize) {
        this->boundingBoxSize = boundingBoxSize;
    }

    void Setup() {
        this->shader.Setup();
        auto pair = this->LoadMeshfile(this->meshPath, boundingBoxSize);
        std::vector<std::array<double, 3>> vertices = pair.first;
        std::vector<std::array<int, 3>> faces = pair.second;

        this->curr = 0;
        this->meshs.resize(1);
        mesh_t &mesh = this->meshs[0];
        mesh.vertices.resize(vertices.size());
        mesh.faces.resize(faces.size());
        mesh.edges.resize(faces.size() * 3);
        for (size_t i = 0; i < vertices.size(); i++) {
            std::array<double, 3> positions = vertices[i];
            vertex_t &vertex = mesh.vertices[i];
            vertex.position.x = positions[0];
            vertex.position.y = positions[1];
            vertex.position.z = positions[2];
            vertex.ID = mesh.uuid_v++;
        }

        std::map<const vertex_t *, bool> associated;
        for (size_t i = 0; i < faces.size(); i++) {
            std::array<int, 3> vertexs = faces[i];
            vertex_t &vertex0 = mesh.vertices[vertexs[0]];
            vertex_t &vertex1 = mesh.vertices[vertexs[1]];
            vertex_t &vertex2 = mesh.vertices[vertexs[2]];

            face_t &face = mesh.faces[i];
            edge_t &edge0 = mesh.edges[i * 3 + 0];
            edge_t &edge1 = mesh.edges[i * 3 + 1];
            edge_t &edge2 = mesh.edges[i * 3 + 2];

            face.edge = &edge0;
            edge0.vertex = &vertex0;
            edge1.vertex = &vertex1;
            edge2.vertex = &vertex2;
            if (!associated[&vertex0]) {
                vertex0.belong = &edge0;
                associated[&vertex0] = true;
            }
            if (!associated[&vertex1]) {
                vertex1.belong = &edge1;
                associated[&vertex1] = true;
            }
            if (!associated[&vertex2]) {
                vertex2.belong = &edge2;
                associated[&vertex2] = true;
            }

            edge0.next = &edge1;
            edge1.next = &edge2;
            edge2.next = &edge0;
            edge0.prev = &edge2;
            edge1.prev = &edge0;
            edge2.prev = &edge1;
            edge0.belong = &face;
            edge1.belong = &face;
            edge2.belong = &face;
            face.ID = mesh.uuid_f++;
            edge0.ID = mesh.uuid_e++;
            edge1.ID = mesh.uuid_e++;
            edge2.ID = mesh.uuid_e++;
        }

        this->setNormal(mesh);
        this->findTwin(mesh, mesh.faces.size() * 3);
        this->setCrease(mesh);

        log_debug("Mesh %d vertices %d faces", vertices.size(), faces.size());
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
                this->LoopSubdivision(oldMesh, newMesh);
            }
        }
        this->curr = targetIndex;
        this->OffloadCurrentMesh();
    }

    void OffloadCurrentMesh()
    {
        mesh_t &mesh = this->meshs[this->curr];
        std::vector<GLfloat> fdata;
        for (size_t i = 0; i < mesh.faces.size(); i++) {
            face_t &face = mesh.faces[i];
            edge_t &edge0 = *face.edge;
            edge_t &edge1 = *edge0.next;
            edge_t &edge2 = *edge0.prev;
            vertex_t &vertex0 = *edge0.vertex;
            vertex_t &vertex1 = *edge1.vertex;
            vertex_t &vertex2 = *edge2.vertex;

            fdata.push_back(vertex0.position.x); fdata.push_back(vertex0.position.y); fdata.push_back(vertex0.position.z);
            fdata.push_back(vertex1.position.x); fdata.push_back(vertex1.position.y); fdata.push_back(vertex1.position.z);
            fdata.push_back(vertex2.position.x); fdata.push_back(vertex2.position.y); fdata.push_back(vertex2.position.z);
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

    virtual void update(double now, double lastUpdateTime, GLFWwindow *window)
    {
        if (this->oldKeyQState == GLFW_PRESS && this->nowKeyQState == GLFW_RELEASE) {
            this->TrySwitchMesh(-1);
        }
        if (this->oldKeyEState == GLFW_PRESS && this->nowKeyEState == GLFW_RELEASE) {
            this->TrySwitchMesh(+1);
        }
    }

    virtual void render(double now, double lastRenderTime, const glm::mat4 &view, const glm::mat4 &projection) {
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


    void LoopSubdivision(mesh_t &coarse, mesh_t &fine)
    {
        std::map<const edge_t *, vertex_t *> odds;
        std::map<const vertex_t *, vertex_t *> evens;

        // old vertex
        for (int i = 0; i < coarse.edges.size(); i++) {
            edge_t &edge = coarse.edges[i];
            if (odds[&edge])
                continue;
            fine.vertices.resize(fine.vertices.size() + 1);
            vertex_t &insertV = fine.vertices.back();
            insertV.position.x = this->loopFomular(&edge, 'x');
            insertV.position.y = this->loopFomular(&edge, 'y');
            insertV.position.z = this->loopFomular(&edge, 'z');
            insertV.ID = fine.uuid_v++;
            odds[&edge] = &insertV;
            if (edge.twin != nullptr)
                odds[edge.twin] = &insertV;
        }
        // even vertex
        uint32_t vertex_count_old = 0;
        for (int i = 0; i < coarse.vertices.size(); i++) {
            vertex_t &vertex = coarse.vertices[i];
            edge_t *find = vertex.belong;
            std::vector<vertex_t> neighbour;
            uint32_t valences = 0;
            do {
                valences += 1;
                neighbour.push_back(*odds[find]);
                if (find->twin == nullptr) {
                    edge_t *findBack = vertex.belong->prev;
                    while (findBack != nullptr) {
                        valences += 1;
                        neighbour.push_back(*odds[findBack]);
                        if (findBack->twin == nullptr)
                            break;
                        findBack = findBack->twin->prev;
                    }
                    break;
                }
                find = find->twin->next;
            } while (find != vertex.belong);

            vertex_t vertexUpdate;
            vertexUpdate = vertex;
            vertexUpdate.position.x = this->adjustFomular(&vertexUpdate, valences, 'x', neighbour, odds);
            vertexUpdate.position.y = this->adjustFomular(&vertexUpdate, valences, 'y', neighbour, odds);
            vertexUpdate.position.z = this->adjustFomular(&vertexUpdate, valences, 'z', neighbour, odds);
            vertexUpdate.ID = fine.uuid_v++;
            fine.vertices.push_back(vertexUpdate);
            evens[&coarse.vertices[i]] = &fine.vertices.back();
        }
        // remesh
        for (int i = 0; i < coarse.faces.size(); i++) {
            face_t &face = coarse.faces[i];
            edge_t e[4][3];
            edge_t &e_origin = *face.edge;
            edge_t &e_previous = *e_origin.prev;
            edge_t &e_next = *e_origin.next;

            e[0][0].vertex = odds[&e_origin];
            e[0][1].vertex = evens[e_next.vertex];
            e[0][2].vertex = odds[&e_next];

            e[1][0].vertex = odds[&e_next];
            e[1][1].vertex = evens[e_previous.vertex];
            e[1][2].vertex = odds[&e_previous];

            e[2][0].vertex = odds[&e_previous];
            e[2][1].vertex = evens[e_origin.vertex];
            e[2][2].vertex = odds[&e_origin];

            e[3][0].vertex = odds[&e_origin];
            e[3][1].vertex = odds[&e_next];
            e[3][2].vertex = odds[&e_previous];

            for (uint32_t i = 0; i < 4; i++) {
                fine.edges.push_back(e[i][0]);
                fine.edges.push_back(e[i][1]);
                fine.edges.push_back(e[i][2]);
                fine.faces.resize(fine.faces.size() + 1);
                edge_t &e0 = fine.edges[fine.edges.size() - 3];
                edge_t &e1 = fine.edges[fine.edges.size() - 2];
                edge_t &e2 = fine.edges[fine.edges.size() - 1];
                face_t &fine_face = fine.faces.back();
                e0.belong = &fine_face;
                e1.belong = &fine_face;
                e2.belong = &fine_face;
                e0.next = &e1;
                e1.next = &e2;
                e2.next = &e0;
                e0.prev = &e2;
                e1.prev = &e0;
                e2.prev = &e1;
                fine_face.edge = &e0;
            }

            odds[&e_origin]->belong = &fine.edges[fine.edges.size() - 12];
            odds[&e_next]->belong = &fine.edges[fine.edges.size() - 10];
            odds[&e_previous]->belong = &fine.edges[fine.edges.size() - 7];
            evens[e_next.vertex]->belong = &fine.edges[fine.edges.size() - 11];
            evens[e_previous.vertex]->belong = &fine.edges[fine.edges.size() - 8];
            evens[e_origin.vertex]->belong = &fine.edges[fine.edges.size() - 5];
        }

        this->findTwin(fine, fine.edges.size());
        this->setNormal(fine);
        this->setCrease(fine);
    }

    double loopFomular(edge_t *insertEdge, char direction)
    {
        if (insertEdge->twin == nullptr || insertEdge->crease) {
            vertex_t *a = insertEdge->vertex;
            vertex_t *b = insertEdge->next->vertex;
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
            vertex_t *a = insertEdge->vertex;
            vertex_t *b = insertEdge->next->vertex;
            vertex_t *c = insertEdge->prev->vertex;
            vertex_t *d = insertEdge->twin->prev->vertex;
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

    double adjustFomular(vertex_t *v, uint32_t valence, char direction, std::vector<vertex_t> neighbour, std::map<const edge_t *, vertex_t *> &odds)
    {
        edge_t *edge = v->belong;
        if (edge->twin == nullptr || edge->crease) {
            vertex_t *a = odds[edge];
            vertex_t *b = odds[edge->prev];
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
    void findTwin(mesh_t &mesh, uint32_t edge_count)
    {
        for (uint32_t i = 0; i < edge_count; i++)
            for (uint32_t j = i + 1; j < edge_count; j++) {
                if (((mesh.edges)[i]).twin)
                    break;
                if (((mesh.edges)[j]).twin)
                    continue;
                if (((mesh.edges)[i]).vertex == (*((mesh.edges)[j]).next).vertex && ((mesh.edges)[j]).vertex == (*((mesh.edges)[i]).next).vertex) {
                    ((mesh.edges)[i]).twin = &(mesh.edges)[j];
                    ((mesh.edges)[j]).twin = &(mesh.edges)[i];
                }
            }
    }
    //compute normal
    void setNormal(mesh_t &mesh)
    {
        uint32_t i = 0;
        // log_trace("total face %d", (*mesh).faces.size());
        for (auto it = mesh.faces.begin(); it != mesh.faces.end(); it++, i++) {
            // log_trace("face %p(%u)", it, (*it).ID);
            edge_t *vertex = (*it).edge;
            // log_trace("    boundary %p(%u)", vertex, (*vertex).ID);
            // v1 is first vector of the face, v2 is the second vector, vn is the normal
            glm::vec3 v1, v2, vn;
            vertex_t &p0 = *(*vertex).vertex;
            // log_trace("    p0 %p(%u)", &p0, p0.ID);
            vertex_t &p1 = *(*(*vertex).next).vertex;
            // log_trace("    p1 %p(%u)", &p1, p1.ID);
            vertex_t &p2 = *(*(*vertex).prev).vertex;
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

    void setCrease(mesh_t &mesh)
    {
        std::map<const edge_t *, bool> checked;
        for (int i= 0; i < mesh.edges.size(); i++) {
            edge_t *edge = &mesh.edges[i];
            if (checked[edge])
                continue;

            glm::vec3 normal0 = glm::normalize(edge->belong->normal);
            glm::vec3 normal1 = glm::normalize(edge->twin->belong->normal);
            double degrees = glm::degrees(glm::acos(glm::dot(normal0, normal1)));

            if (degrees >= 150) {
                edge->crease = true;
                edge->twin->crease = true;
            } else {
                edge->crease = false;
                edge->twin->crease = false;
            }
            checked[edge] = true;
            checked[edge->twin] = true;
        }
    }

    std::pair<std::vector<std::array<double, 3>>, std::vector<std::array<int, 3>>>
    LoadMeshfile(const std::string &path, const double boundingBoxSize) {
        std::vector<std::array<double, 3>> vertices;
        std::vector<std::array<int, 3>> faces;
        
        std::ifstream fin;
        std::ifstream::iostate old_state = fin.exceptions();
        fin.exceptions(std::ifstream::badbit | std::ifstream::failbit);
        try {
            fin.open(path, std::ios::in);
        } catch (std::ifstream::failure& e) {
            log_error("Mesh(%s) was not successfully read: %s", path.c_str(), e.what());
            return std::make_pair(vertices, faces);
        }
        fin.exceptions(old_state);

        int offset = 0;
        if (path.find(".0.mesh") != std::string::npos) {
            offset = 0;
        } else if (path.find(".1.mesh") != std::string::npos) {
            offset = 1;
        } else {
            log_warn("File %s extension is not \".0.mesh\" or \".1.mesh\", "
                "assuming face index start from 1", path.c_str()
            );
            offset = 0;
        }

        std::array<double, 3> mincoords = {1e100, 1e100, 1e100};
        std::array<double, 3> maxcoords = {-1e100, -1e100, -1e100};
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
                // log_trace("%f %f %f", mincoords[0], mincoords[1], mincoords[2]);
                // log_trace("%f %f %f", positions[0], positions[1], positions[2]);
                mincoords[0] = std::min(mincoords[0], positions[0]);
                mincoords[1] = std::min(mincoords[1], positions[1]);
                mincoords[2] = std::min(mincoords[2], positions[2]);
                maxcoords[0] = std::max(maxcoords[0], positions[0]);
                maxcoords[1] = std::max(maxcoords[1], positions[1]);
                maxcoords[2] = std::max(maxcoords[2], positions[2]);
                vertices.push_back(std::move(positions));
            } else if (t == 'f') {
                std::array<int, 3> indexs;
                instring >> indexs[0] >> indexs[1] >> indexs[2];
                indexs[0] -= offset;
                indexs[1] -= offset;
                indexs[2] -= offset;
                faces.push_back(std::move(indexs));
            }
        }

        double bounding = -1e100;
        bounding = std::max(bounding, maxcoords[0] - mincoords[0]);
        bounding = std::max(bounding, maxcoords[1] - mincoords[1]);
        bounding = std::max(bounding, maxcoords[2] - mincoords[2]);
        log_trace("%f %f %f", mincoords[0], mincoords[1], mincoords[2]);
        log_trace("%f %f %f", maxcoords[0], maxcoords[1], maxcoords[2]);
        assert(bounding != 0);
        double scale = boundingBoxSize / bounding;
        for (int i = 0; i < vertices.size(); i++) {
            vertices[i][0] *= scale;
            vertices[i][1] *= scale;
            vertices[i][2] *= scale;
        }

        fin.close();
        return std::make_pair(vertices, faces);
    }

    virtual void refresh(double now, double lastTime, GLFWwindow *window)
    {
        int width = 0;
        int height = 0;
        glfwGetWindowSize(window, &width, &height);
        ImGui::SetNextWindowPos(ImVec2(width - 360, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::Begin("Tips", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
        ImGui::Text("Press Q to switch to coarse mesh");
        ImGui::Text("Press E to switch to fine mesh (Loop Subdivision)");
        ImGui::Text("Current mesh index: %d of %d", (int)this->curr+1, (int)this->meshs.size());
        ImGui::End();
    }

};
