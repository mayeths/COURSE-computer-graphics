#include <GLFW/glfw3.h>
#include <SOIL.h>
#include <glad/glad.h>
#include <stdio.h>
#include <string.h>

#include <array>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "log.h"
#include "Scene.hpp"
#include "Window.hpp"
#include "GUI.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "SmileBox.hpp"
#include "zyMesh.h"

#include "halfEdge_structure.h"

// std::string fireHead = "";
// std::string faceStart = "";
// std::string fireEnd = "";
// unsigned vertexNumber = 0;
// unsigned faceNumber = 0;
// unsigned halfedgeNumber = 0;

// //check the head,just for this file version
// bool checkHead(string s)
// {
// 	return(s == "#VRMLV2.0utf8(ConvertedtoASCII)Shape{geometryIndexedFaceSet{coordCoordinate{point[");
// }

// //check string between vertexs and faces
// bool checkFace(string s)
// {
// 	return(s == "}coordIndex[");
// }

// //check end
// bool checkEnd(string s)
// {
// 	return(s == "}}");
// }

// //convert string to double
// double stringToDouble(const string& str)
// {
// 	istringstream iss(str);
// 	double num;
// 	iss >> num;
// 	return num;
// }

// //convert string to int
// int stringToInt(const string& str)
// {
// 	istringstream iss(str);
// 	int num;
// 	iss >> num;
// 	return num;
// }
// unsigned countLine(string file)
// {
// 	ifstream wrlFile;
// 	unsigned a = 0;
// 	wrlFile.open(file.data());   //connect file stream
// 	assert(wrlFile.is_open());   //if fail, warning and exit
// 	string tempLine;
	
// 	while (getline(wrlFile,tempLine)) a++;
// 	return a;
// }

// //read file, generate vertex table, halfedge table and face table
// int readWrl(string file, HalfEdge_mesh* mesh)
// {
//     ifstream wrlFile;
//     wrlFile.open(file.data());   //connect file stream
//     assert(wrlFile.is_open());   //if fail, warning and exit

//     HalfEdge_vertex vertex_temp;
//     unsigned count_vertex_read = 0;
//     unsigned count_face_read = 0;
//     unsigned count_halfedge_read = 0;
//     int edge[3];
//     string sTemp;
//     bool headChecked = false;
//     bool vertexEnd = false;
//     bool pointSettled = false;
//     bool dataSettled = false;
//     while (wrlFile >> sTemp)
//     {
//         //head check
//         if (!headChecked) {
//             if ("[" == sTemp || "point[" == sTemp) {
//                 fireHead += sTemp;
//                 if (checkHead(fireHead)) {
//                     headChecked = true;
//                     continue;
//                 }
//                 else return -1;
//             }
//             else fireHead += sTemp;
//         }
//         //set point coordinate
//         if (headChecked && !pointSettled) {
//             if (vertexEnd) {
//                 faceStart += sTemp;
//                 if (checkFace(faceStart))
//                     pointSettled = true;
//                 else if (wrlFile.eof() && !checkFace(faceStart))
//                     return -2;
//                 continue;
//             }

//             if ("]" == sTemp) {
//                 vertexEnd = true;
//                 continue;
//             }
//             if (0 == count_vertex_read % 3)
//                 vertex_temp.x = stringToDouble(sTemp);
//             if (1 == count_vertex_read % 3)
//                 vertex_temp.y = stringToDouble(sTemp);
//             if (2 == count_vertex_read % 3) {
//                 vertex_temp.z = stringToDouble(sTemp);
//                 (*mesh).vertex.push_back(vertex_temp);
//                 (*mesh).vertex[count_vertex_read / 3].number = count_vertex_read / 3;
//             }
//             count_vertex_read++;
//         }
//         //set face and halfedge
//         if (pointSettled && !dataSettled){
//             if ("]" == sTemp) {
//                 findTwin(mesh,count_halfedge_read);
//                 //findOrigin(vector<HalfEdge_halfedge> halfedge, vector<HalfEdge_vertex> vertex);
//                 dataSettled = true;
//                 continue;
//             }
//             else if ("-1" == sTemp) {
//                 HalfEdge_halfedge edge_temp1;
//                 HalfEdge_halfedge edge_temp2;
//                 HalfEdge_halfedge edge_temp3;
//                 HalfEdge_face face_temp;
//                 (*mesh).halfedge.push_back(edge_temp1);
//                 (*mesh).halfedge.push_back(edge_temp2);
//                 (*mesh).halfedge.push_back(edge_temp3);
//                 //face
//                 (*mesh).face.push_back(face_temp);
//                 (*mesh).face[count_face_read].boundary = &(*mesh).halfedge[count_halfedge_read - 3];
//                 //halfedge's origin & vertexs
//                 (*mesh).halfedge[count_halfedge_read - 3].origin = &(*mesh).vertex[edge[0]];
//                 if (!(*mesh).vertex[edge[0]].beenOrigined) {
//                     (*mesh).vertex[edge[0]].asOrigin = &(*mesh).halfedge[count_halfedge_read - 3];
//                     (*mesh).vertex[edge[0]].beenOrigined = true;
//                 }
//                 ((*mesh).halfedge[count_halfedge_read - 2]).origin = &(*mesh).vertex[edge[1]];
//                 if (!(*mesh).vertex[edge[1]].beenOrigined) {
//                     (*mesh).vertex[edge[1]].asOrigin = &(*mesh).halfedge[count_halfedge_read - 2];
//                     (*mesh).vertex[edge[1]].beenOrigined = true;
//                 }
//                 (*mesh).halfedge[count_halfedge_read - 1].origin = &(*mesh).vertex[edge[2]];
//                 if (!(*mesh).vertex[edge[2]].beenOrigined){
//                     (*mesh).vertex[edge[2]].asOrigin = &(*mesh).halfedge[count_halfedge_read - 1];
//                     (*mesh).vertex[edge[2]].beenOrigined = true;
//                 }
//                 //halfedge's nextedge
//                 (*mesh).halfedge[count_halfedge_read - 3].nextEdge = &(*mesh).halfedge[count_halfedge_read - 2];
//                 (*mesh).halfedge[count_halfedge_read - 2].nextEdge = &(*mesh).halfedge[count_halfedge_read - 1];
//                 (*mesh).halfedge[count_halfedge_read - 1].nextEdge = &(*mesh).halfedge[count_halfedge_read - 3];
//                 //halfedge's preedge
//                 (*mesh).halfedge[count_halfedge_read - 3].preEdge = &(*mesh).halfedge[count_halfedge_read - 1];
//                 (*mesh).halfedge[count_halfedge_read - 2].preEdge = &(*mesh).halfedge[count_halfedge_read - 3];
//                 (*mesh).halfedge[count_halfedge_read - 1].preEdge = &(*mesh).halfedge[count_halfedge_read - 2];
//                 //halfedge's incidentface
//                 (*mesh).halfedge[count_halfedge_read - 3].incidentFace = &(*mesh).face[count_face_read];
//                 (*mesh).halfedge[count_halfedge_read - 2].incidentFace = &(*mesh).face[count_face_read];
//                 (*mesh).halfedge[count_halfedge_read - 1].incidentFace = &(*mesh).face[count_face_read];
//                 //halfedge's name
//                 (*mesh).halfedge[count_halfedge_read - 3].number = count_halfedge_read - 3;
//                 (*mesh).halfedge[count_halfedge_read - 2].number = count_halfedge_read - 2;
//                 (*mesh).halfedge[count_halfedge_read - 1].number = count_halfedge_read - 1;
//                 count_face_read++;
//             }
//             //a set contains three vertexs
//             else if (0 == count_halfedge_read % 3) {
//                 edge[0] = stringToInt(sTemp);
//                 count_halfedge_read++;
//             }
//             else if (1 == count_halfedge_read % 3) {
//                 edge[1] = stringToInt(sTemp);
//                 count_halfedge_read++;
//             }
//             else if (2 == count_halfedge_read % 3) {
//                 edge[2] = stringToInt(sTemp);
//                 count_halfedge_read++;
//             }
//         }
//         //end check
//         if (dataSettled) {
//             fireEnd += sTemp;
//             if (checkEnd(fireEnd)) {
//                 setNormal(mesh);
//                 setCrease(mesh);
//                 break;
//             }
//             else if (wrlFile.eof() && !checkEnd(fireEnd))
//                 return -3;
//         }
//     }
//     halfedgeNumber = count_halfedge_read;
//     vertexNumber = count_vertex_read / 3;
//     faceNumber = count_face_read;
//     wrlFile.close();
//     return 0;
// }

int main() {
    GLint success = 0;
    Window window(&success, "A-4-LoopSubdivision", 800, 600);
    if (success != 1) {
        log_fatal("Failed to create GLFW window");
        return -1;
    }

    std::vector<SmileBox> smileBoxs(12);
    for (int i = 0; i < smileBoxs.size(); i++) {
        smileBoxs[i].SetShaderPath("assets/shaders/smilebox.vs", "assets/shaders/smilebox.fs");
        smileBoxs[i].SetTexturePath("assets/textures/container.jpg", "assets/textures/awesomeface.png");
        smileBoxs[i].Setup(i);

        std::vector<glm::vec3> cubePositions {
            glm::vec3(0.0f * 2, -1.0f, -3.0f),
            glm::vec3(1.0f * 2, -1.0f, -3.0f),
            glm::vec3(2.0f * 2, -1.0f, -3.0f),
            glm::vec3(3.0f * 2, -1.0f, -3.0f),
            glm::vec3(0.0f * 2,  0.0f, -3.0f),
            glm::vec3(1.0f * 2,  0.0f, -3.0f),
            glm::vec3(2.0f * 2,  0.0f, -3.0f),
            glm::vec3(3.0f * 2,  0.0f, -3.0f),
            glm::vec3(0.0f * 2, +1.0f, -3.0f),
            glm::vec3(1.0f * 2, +1.0f, -3.0f),
            glm::vec3(2.0f * 2, +1.0f, -3.0f),
            glm::vec3(3.0f * 2, +1.0f, -3.0f),
        };
        assert(smileBoxs.size() <= cubePositions.size());
        smileBoxs[i].MoveTo(cubePositions[i]);
    }

    Camera &camera = window.camera;
    {
        camera.setPosition(glm::vec3(0.0f, 0.0f, 3.0f));
        camera.setMovementSpeed(15);
        camera.setMaxRenderDistance(1e8f);
    }

    Mesh mesh;
    mesh.SetMeshPath("assets/meshs/wrlcube.obj");
    mesh.SetShaderPath("assets/shaders/mesh.vs", "assets/shaders/mesh.fs");
    mesh.Setup();

    GUI gui(window);
    {
        gui.subscribe([&](GLFWwindow *w, double lastRenderTime, double now) {
            ImGui::SetNextWindowPos(ImVec2(window.SCR_WIDTH-60, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::Begin("Stats", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SameLine();
            ImGui::Text("%.0f FPS", 1 / (now - lastRenderTime));
            ImGui::End();
        });
        gui.subscribe([&](GLFWwindow *w, double lastRenderTime, double now) {
            glm::vec3 cameraPos = camera.Position;
            ImGui::SetNextWindowPos(ImVec2(0, 44), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
            ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::Begin("Tips", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::Text("Position %2.0f %2.0f %2.0f", cameraPos[0], cameraPos[1], cameraPos[2]);
            ImGui::Text("Press Tab to enter god mod");
            ImGui::End();
        });
    }

    double now;
    double lastUpdateTime = 0;
    double lastRenderTime = 0;
    while (window.continueLoop()) {
        lastRenderTime = now;
        lastUpdateTime = now;
        now = glfwGetTime();
        double deltaUpdateTime = now - lastUpdateTime;
        double deltaRenderTime = now - lastRenderTime;

        ////// Update Logic
        window.processInput(deltaUpdateTime, deltaRenderTime);
        mesh.update(now, deltaUpdateTime);
        gui.update();

        ////// Render Frame
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float screenRatio = (float)window.SCR_WIDTH / (float)window.SCR_HEIGHT;
        glm::mat4 projection = camera.GetProjectionMatrix(lastRenderTime, now, screenRatio);
        glm::mat4 view = camera.GetViewMatrix();
        // render object first
        for (int i = 0; i < smileBoxs.size(); i++)
            smileBoxs[i].render(now, deltaRenderTime, view, projection);
        mesh.render(now, deltaRenderTime, view, projection);
        // finally render GUI
        gui.render(window.w, lastRenderTime, now);

        ////// Finish Render
        window.swapBuffersAndPollEvents();
    }

    return 0;
}
