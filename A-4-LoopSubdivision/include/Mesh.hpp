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
    edge_t *belong = nullptr; // Edge which starts from this vertex
};

struct edge_t {
    uint32_t ID = 0;
    bool crease = false; // A crease (or boundary)
    vertex_t *vertex = nullptr;
    edge_t *twin = nullptr;
    edge_t *next = nullptr;
    edge_t *prev = nullptr;
    face_t *belong = nullptr;
};

struct face_t {
    uint32_t ID = 0;
    glm::vec3 normal = glm::vec3(0, 0, 0);
    edge_t *edge = nullptr; // Edge of this face
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
    bool enableGUI = true;

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
    void EnableGUI(bool enableGUI)
    {
        this->enableGUI = enableGUI;
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

        this->SetNormal(mesh);
        this->FindTwin(mesh);
        this->SetCrease(mesh);

        log_debug("Mesh \"%s\" has %d vertices %d faces", this->meshPath.c_str(), vertices.size(), faces.size());
        this->OffloadCurrentMesh();
        this->guard.set();
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

    void LoopSubdivision(mesh_t &coarse, mesh_t &fine)
    {
        std::map<const edge_t *, vertex_t *> odds;
        std::map<const vertex_t *, vertex_t *> evens;

        // odd vertex
        for (int i = 0; i < coarse.edges.size(); i++) {
            edge_t &edge = coarse.edges[i];
            if (odds[&edge])
                continue;
            fine.vertices.resize(fine.vertices.size() + 1);
            vertex_t &odd = fine.vertices.back();
            odd.position = this->ComputeOddPosition(edge);
            odd.ID = fine.uuid_v++;
            odds[&edge] = &odd;
            if (edge.twin != nullptr)
                odds[edge.twin] = &odd;
        }
        // even vertex
        uint32_t vertex_count_old = 0;
        for (int i = 0; i < coarse.vertices.size(); i++) {
            vertex_t &vertex = coarse.vertices[i];
            edge_t *find = vertex.belong;
            std::vector<vertex_t> neighbour;
            do {
                neighbour.push_back(*odds[find]);
                if (find->twin == nullptr) {
                    edge_t *findBack = vertex.belong->prev;
                    while (findBack != nullptr) {
                        neighbour.push_back(*odds[findBack]);
                        if (findBack->twin == nullptr)
                            break;
                        findBack = findBack->twin->prev;
                    }
                    break;
                }
                find = find->twin->next;
            } while (find != vertex.belong);

            vertex_t even;
            even = vertex;
            even.position = this->RecomputeEvenPosition(even, odds, neighbour);
            even.ID = fine.uuid_v++;
            fine.vertices.push_back(even);
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

        this->FindTwin(fine);
        this->SetNormal(fine);
        this->SetCrease(fine);
        log_debug("Mesh \"%s\" subdivision %d vertices %d faces",
            this->meshPath.c_str(), fine.vertices.size(), fine.faces.size()
        );
    }

    glm::vec3 ComputeOddPosition(const edge_t &edge)
    {
        if (edge.twin == nullptr || edge.crease) {
            vertex_t *a = edge.vertex;
            vertex_t *b = edge.next->vertex;
            return (a->position + b->position) / 2.0f;
        } else {
            vertex_t *a = edge.vertex;
            vertex_t *b = edge.next->vertex;
            vertex_t *c = edge.prev->vertex;
            vertex_t *d = edge.twin->prev->vertex;
            return (3.0f / 8.0f) * (a->position + b->position) + (1.0f / 8.0f) * (c->position + d->position);
        }
    }

    glm::vec3 RecomputeEvenPosition(vertex_t &even, std::map<const edge_t *, vertex_t *> &odds, const std::vector<vertex_t> &neighbour)
    {
        edge_t *edge = even.belong;
        if (edge->twin == nullptr || edge->crease) {
            vertex_t *a = odds[edge];
            vertex_t *b = odds[edge->prev];
            return (1.0f / 8.0f) * (a->position + b->position) + (3.0f / 4.0f) * even.position;
        } else {
            float n = (float)neighbour.size();
            float beta = 0;
            /* Loop's suggestion for beta */
            float x = (3.0 / 8.0 + 1.0 / 4.0 * cos(2.0 * M_PI / n));
            beta = (1.0 / n) * (5.0 / 8.0 - x * x);
            /* Warren's suggestion for beta */
            // if (neighbour.size() == 3)
            //     beta = 3.0 / 16.0;
            // else if (neighbour.size() > 3)
            //     beta = 3.0 / (8.0 * n);
            // else
            //     throw std::runtime_error("you should not come here 4.0.0");
            glm::vec3 sum = glm::vec3(0.0f, 0.0f, 0.0f);
            for (int i = 0; i < neighbour.size(); i++)
                sum += neighbour[i].position;
            return even.position * (1.0f - n * beta) + sum * beta;
        }
    }

    void FindTwin(mesh_t &mesh)
    {
        for (size_t i = 0; i < mesh.edges.size(); i++) {
            edge_t &ei = mesh.edges[i];
            for (size_t j = i + 1; j < mesh.edges.size(); j++) {
                edge_t &ej = mesh.edges[j];
                if (ei.twin)
                    break;
                if (ej.twin)
                    continue;
                if (ei.vertex == ej.next->vertex && ej.vertex == ei.next->vertex) {
                    ei.twin = &ej;
                    ej.twin = &ei;
                }
            }
        }
    }

    void SetNormal(mesh_t &mesh)
    {
        for (size_t i = 0; i < mesh.faces.size(); i++) {
            face_t &face = mesh.faces[i];
            glm::vec3 p0 = face.edge->vertex->position;
            glm::vec3 p1 = face.edge->next->vertex->position;
            glm::vec3 p2 = face.edge->prev->vertex->position;
            glm::vec3 normal = glm::cross(p0 - p1, p0 - p2);
            if (glm::length(normal) == 0.0)
                throw std::runtime_error("A normal length is 0");
            face.normal = glm::normalize(normal);
        }
    }

    void SetCrease(mesh_t &mesh)
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
        double scale = boundingBoxSize / bounding;
        for (int i = 0; i < vertices.size(); i++) {
            vertices[i][0] *= scale;
            vertices[i][1] *= scale;
            vertices[i][2] *= scale;
        }

        fin.close();
        return std::make_pair(vertices, faces);
    }

    virtual void Update(double now, double lastUpdateTime, GLFWwindow *window)
    {
        this->oldKeyQState = this->nowKeyQState;
        this->oldKeyEState = this->nowKeyEState;
        this->nowKeyQState = glfwGetKey(window, GLFW_KEY_Q);
        this->nowKeyEState = glfwGetKey(window, GLFW_KEY_E);
        if (this->oldKeyQState == GLFW_PRESS && this->nowKeyQState == GLFW_RELEASE) {
            this->TrySwitchMesh(-1);
        }
        if (this->oldKeyEState == GLFW_PRESS && this->nowKeyEState == GLFW_RELEASE) {
            this->TrySwitchMesh(+1);
        }
    }

    virtual void Render(double now, double lastRenderTime, const glm::mat4 &view, const glm::mat4 &projection) {
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

    virtual void RenderGUI(double now, double lastTime, GLFWwindow *window)
    {
        if (!this->enableGUI)
            return;
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
