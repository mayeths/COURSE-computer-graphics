#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdio.h>
#include <string.h>
#include <stb_image.h>

#include <array>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "log.h"
#include "Scene.hpp"
#include "Window.hpp"
#include "GUI.hpp"
#include "Shader.hpp"
#include "SkyBox.hpp"
#include "SmileBox.hpp"
#include "Terrian.hpp"

int main() {
    GLint success = 0;
    Window window(&success, "A-6-TerrianEngine", 800, 600);
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
        camera.setPosition(glm::vec3(0.0f, 10.0f, 200.0f));
        camera.setMaxRenderDistance(1e8f);
    }

    Scene scene;
    SkyBox skybox;
    {
        skybox.SetShaderPath("assets/shaders/skybox.vs", "assets/shaders/skybox.fs");
        skybox.SetTopImagePath("assets/textures/SkyBox4.bmp");
        skybox.SetNorthImagePath("assets/textures/SkyBox0.bmp");
        skybox.SetEastImagePath("assets/textures/SkyBox1.bmp", -90);
        skybox.SetSouthImagePath("assets/textures/SkyBox2.bmp");
        skybox.SetWestImagePath("assets/textures/SkyBox3.bmp", -90);
        skybox.SetBottomImagePath("assets/textures/SkyBox5.bmp");
        GLfloat skyBoxWidth = 1e4f; // Big value (likes 1e5f) will meet precision limit when testing depth
        skybox.SetBoxWidth(skyBoxWidth);
        skybox.MoveWith(glm::vec3(0.0f, skyBoxWidth/2, 0.0f));
        skybox.Setup();
    }

    Terrian terrian;
    {
        terrian.SetShaderPath(
            "assets/shaders/terrian.vs", "assets/shaders/terrian.fs",
            "assets/shaders/terrian.tcs", "assets/shaders/terrian.tes"
        );
        terrian.SetHeightMapPath("assets/textures/heightmap.bmp");
        terrian.SetTexturePath("assets/textures/terrain-texture3.bmp");
        terrian.SetDetailTexturePath("assets/textures/detail.bmp");
        terrian.MoveWith(glm::vec3(0.0f, -2.0f, 0.0f));
        terrian.Setup();
    }

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
        gui.subscribe(&terrian);
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
        skybox.update(now, deltaUpdateTime);
        terrian.update(now, deltaUpdateTime);
        gui.update();

        ////// Render Frame
        glClearColor(0, 0, 0., 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float screenRatio = (float)window.SCR_WIDTH / (float)window.SCR_HEIGHT;
        glm::mat4 projection = camera.GetProjectionMatrix(lastRenderTime, now, screenRatio);
        glm::mat4 view = camera.GetViewMatrix();
        // render object first
        for (int i = 0; i < smileBoxs.size(); i++)
            smileBoxs[i].render(now, deltaRenderTime, view, projection);
        terrian.render(now, deltaRenderTime, view, projection);
        // then render transparent object (water)
        skybox.render(now, deltaRenderTime, view, projection);
        // finally render GUI
        gui.render(window.w, lastRenderTime, now);

        ////// Finish Render
        window.swapBuffersAndPollEvents();
    }

    return 0;
}
