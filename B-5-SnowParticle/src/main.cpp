#include <GLFW/glfw3.h>
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

#include "log.h"
#include "Window.hpp"
#include "GUI.hpp"
#include "Shader.hpp"
#include "SnowSystem.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

int main() {
    GLint success = 0;
    Window window(&success, "B-5-SnowParticle", 800, 600);
    if (success != 1) {
        log_fatal("Failed to create GLFW window");
        return -1;
    }

    Camera &camera = window.camera;
    camera.setPosition(glm::vec3(0.0f, 50.0f, 200.0f));
    camera.setMaxRenderDistance(1e6f);

    std::vector<const GLchar *> varyings = {
        "type_1", "position_1", "velocity_1", "age_1", "size_1"
    };
    SnowSystem snowSystem;
    snowSystem.SetUpdateShader(
        "./assets/update.vs", "./assets/update.fs", "./assets/update.gs", varyings
    );
    snowSystem.SetRenderShader("./assets/render.vs", "./assets/render.fs");
    snowSystem.SetTexturePath("./assets/SnowFlake.bmp");
    snowSystem.Setup();

    GUI gui(window);
    gui.subscribe([&](GLFWwindow *w, double lastRenderTime, double now) {
        ImGui::SetNextWindowPos(ImVec2(window.SCR_WIDTH-60, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::Begin("Stats", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        ImGui::SameLine();
        ImGui::Text("%.0f FPS", 1 / (now - lastRenderTime));
        ImGui::End();
    });
    gui.subscribe([&](GLFWwindow *w, double lastRenderTime, double now) {
        ImGui::SetNextWindowPos(ImVec2(0, 44), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::Begin("Tips", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        ImGui::Text("Position %2.0f %2.0f %2.0f",
            camera.Position[0], camera.Position[1], camera.Position[2]
        );
        ImGui::Text("Press Tab to enter god mod");
        ImGui::End();
    });

    double now;
    double lastUpdateTime = 0;
    double lastRenderTime = 0;
    while (window.continueLoop()) {
        lastRenderTime = now;
        lastUpdateTime = now;
        now = glfwGetTime();
        double deltaUpdateTime = now - lastUpdateTime;
        double deltaRenderTime = now - lastRenderTime;

        ////// logic update
        window.processInput(deltaUpdateTime, deltaRenderTime);
        snowSystem.update(now, deltaUpdateTime);
        gui.update();

        ////// frame render
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = camera.GetProjectionMatrix(
            lastRenderTime, now, (float)window.SCR_WIDTH / (float)window.SCR_HEIGHT
        );
        snowSystem.render(now, deltaRenderTime, view, projection);
        gui.render(window.w, lastRenderTime, now);

        window.swapBuffersAndPollEvents();
    }

    return 0;
}
