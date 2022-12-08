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
#define MEMORY_IMPL
#include "memory.h"
#include "Scene.hpp"
#include "Window.hpp"
#include "GUI.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "SmileBox.hpp"

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
            ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::Begin("Stats", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
            ImGui::SameLine();
            ImGui::Text("%2.0f FPS", 1 / (now - lastRenderTime));
            ImGui::Text("%s", readable_size(allocated_bytes).c_str());
            ImVec2 window1_size = ImGui::GetWindowSize();
            ImGui::SetWindowPos(ImVec2(window.SCR_WIDTH - window1_size.x, 0), ImGuiCond_Always);
            ImGui::End();
        });
        gui.subscribe([&](GLFWwindow *w, double lastRenderTime, double now) {
            glm::vec3 cameraPos = camera.Position;
            ImGui::SetNextWindowPos(ImVec2(0, 94), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
            ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::Begin("Tips", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::Text("Position %2.0f %2.0f %2.0f", cameraPos[0], cameraPos[1], cameraPos[2]);
            ImGui::Text("Press Tab to enable/disable mouse");
            ImGui::Text("Press Q to switch to coarse mesh");
            ImGui::Text("Press E to switch to fine mesh (Loop Subdivision)");
            ImGui::Text("Current mesh index: %d of %d", (int)mesh.curr+1, (int)mesh.meshs.size());
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
        mesh.processInput(window.w);
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
