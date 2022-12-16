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


#include "framework/GUI.hpp"
#include "framework/Shader.hpp"
#include "util/log.h"
#include "Window.hpp"
#include "Mesh.hpp"
#include "SmileBox.hpp"

int main() {
    GLint success = 0;
    Window window(&success, "A-4-LoopSubdivision", 800, 600);
    if (success != 1) {
        log_fatal("Failed to create GLFW window");
        return -1;
    }
    window.EnableStatisticGUI(Window::STAT_FPS | Window::STAT_MEMORY | Window::STAT_POSITION);

    std::vector<SmileBox> smileBoxs(12);
    for (int i = 0; i < smileBoxs.size(); i++) {
        smileBoxs[i].SetShaderPath("assets/shaders/smilebox.vs", "assets/shaders/smilebox.fs");
        smileBoxs[i].SetTexturePath("assets/textures/container.jpg", "assets/textures/awesomeface.png");
        smileBoxs[i].Setup(i);

        std::vector<glm::vec3> cubePositions {
            glm::vec3(0.0f * 2, -1.0f, -40.0f),
            glm::vec3(1.0f * 2, -1.0f, -40.0f),
            glm::vec3(2.0f * 2, -1.0f, -40.0f),
            glm::vec3(3.0f * 2, -1.0f, -40.0f),
            glm::vec3(0.0f * 2,  0.0f, -40.0f),
            glm::vec3(1.0f * 2,  0.0f, -40.0f),
            glm::vec3(2.0f * 2,  0.0f, -40.0f),
            glm::vec3(3.0f * 2,  0.0f, -40.0f),
            glm::vec3(0.0f * 2, +1.0f, -40.0f),
            glm::vec3(1.0f * 2, +1.0f, -40.0f),
            glm::vec3(2.0f * 2, +1.0f, -40.0f),
            glm::vec3(3.0f * 2, +1.0f, -40.0f),
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
    mesh.SetMeshPath("assets/meshs/teddy.1.mesh");
    mesh.SetShaderPath("assets/shaders/mesh.vs", "assets/shaders/mesh.fs");
    mesh.SetBoundingBoxSize(10.0);
    mesh.MoveTo(glm::vec3(0, 0, -10));
    mesh.Setup();

    window.AddObject(&mesh);

    double now;
    double lastUpdateTime = 0;
    double lastRenderTime = 0;
    while (window.ContinueLoop()) {
        lastRenderTime = now;
        lastUpdateTime = now;
        now = glfwGetTime();
        double deltaUpdateTime = now - lastUpdateTime;
        double deltaRenderTime = now - lastRenderTime;

        ////// Update Logic
        window.processInput(deltaUpdateTime, deltaRenderTime);
        mesh.processInput(window.w);
        mesh.update(now, lastUpdateTime, window.w);

        ////// Render Frame
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float screenRatio = (float)window.SCR_WIDTH / (float)window.SCR_HEIGHT;
        glm::mat4 projection = camera.GetProjectionMatrix(now, lastRenderTime, screenRatio);
        glm::mat4 view = camera.GetViewMatrix();
        // render object first
        for (int i = 0; i < smileBoxs.size(); i++)
            smileBoxs[i].render(now, lastRenderTime, view, projection);
        mesh.render(now, lastRenderTime, view, projection);
        // finally render GUI
        window.TriggerRender(now, lastRenderTime);

        ////// Finish Render
        window.SwapBuffersAndPollEvents();
    }

    return 0;
}
