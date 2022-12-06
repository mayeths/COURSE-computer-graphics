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
        smileBoxs[i].SetShaderPath("assets/6.2.coordinate_systems.vs", "assets/6.2.coordinate_systems.fs");
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
    camera.setPosition(glm::vec3(0.0f, 10.0f, 200.0f));
    camera.setMaxRenderDistance(1e8f);

    Scene scene;
    SkyBox skybox;
    skybox.SetShaderPath("assets/skybox.vs", "assets/skybox.fs");
    skybox.SetTopImagePath("assets/SkyBox/SkyBox4.bmp");
    skybox.SetNorthImagePath("assets/SkyBox/SkyBox0.bmp");
    skybox.SetEastImagePath("assets/SkyBox/SkyBox1.bmp", -90);
    skybox.SetSouthImagePath("assets/SkyBox/SkyBox2.bmp");
    skybox.SetWestImagePath("assets/SkyBox/SkyBox3.bmp", -90);
    skybox.SetBottomImagePath("assets/SkyBox/SkyBox5.bmp");
    GLfloat skyBoxWidth = 1e4f; // Big value (likes 1e5f) will meet precision limit when testing depth
    skybox.SetBoxWidth(skyBoxWidth);
    skybox.MoveWith(glm::vec3(0.0f, skyBoxWidth/2, 0.0f));
    skybox.Setup();

    Terrian terrian;
    terrian.SetShaderPath(
        "assets/8.3.gpuheight.vs", "assets/8.3.gpuheight.fs",
        "assets/8.3.gpuheight.tcs", "assets/8.3.gpuheight.tes"
    );
    // terrian.SetShaderPath("assets/terrian.vs", "assets/terrian.fs");
    // terrian.SetHeightMapPath("assets/textures/iceland_heightmap.png");
    terrian.SetHeightMapPath("assets/TerrianHW/heightmap.bmp");
    terrian.SetTexturePath("assets/TerrianHW/terrain-texture3.bmp");
    terrian.SetDetailTexturePath("assets/TerrianHW/detail.bmp");
    terrian.MoveWith(glm::vec3(0.0f, -2.0f, 0.0f));
    terrian.Setup();

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
    gui.subscribe(&terrian);

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
        skybox.update(now, deltaUpdateTime);
        terrian.update(now, deltaUpdateTime);
        gui.update();

        ////// frame render
        // glClearColor(64 / 256., 48 / 256., 64 / 256., 1.0f);
        glClearColor(0, 0, 0., 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, texture1);
        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, texture2);

        float screenRatio = (float)window.SCR_WIDTH / (float)window.SCR_HEIGHT;
        glm::mat4 projection = camera.GetProjectionMatrix(lastRenderTime, now, screenRatio);
        glm::mat4 view = camera.GetViewMatrix();

        // ourShader.use();
        for (int i = 0; i < smileBoxs.size(); i++) {
            smileBoxs[i].render(now, deltaRenderTime, view, projection);
        }

        // glBindVertexArray(VAO);
        // for (int i = 0; i < cubePositions.size(); i++) {
        //     glm::mat4 model = glm::mat4(1.0f);
        //     model = glm::translate(model, cubePositions[i]);
        //     model = glm::rotate(model, (float)(45 + now * i), scene.Front());
        //     ourShader.setMat4("model", model);
        //     glDrawArrays(GL_TRIANGLES, 0, 36);
        // }

        terrian.render(now, deltaRenderTime, view, projection); // render object first
        skybox.render(now, deltaRenderTime, view, projection); // then render transparent object
        gui.render(window.w, lastRenderTime, now);

        window.swapBuffersAndPollEvents();
    }

    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);

    return 0;
}
