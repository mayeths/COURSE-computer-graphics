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

    Camera &camera = window.camera;
    {
        camera.SetPosition(glm::vec3(0.0f, 0.0f, 3.0f));
        camera.SetMovementSpeed(15);
        camera.SetMaxRenderDistance(1e8f);
    }

    Mesh cow;
    cow.SetMeshPath("assets/meshs/cow.1.mesh");
    cow.SetShaderPath("assets/shaders/mesh.vs", "assets/shaders/mesh.fs");
    cow.SetBoundingBoxSize(10.0);
    cow.MoveTo(glm::vec3(0, 0, -20));
    cow.Setup();
    window.AddObject(&cow);

    Mesh teddy;
    teddy.SetMeshPath("assets/meshs/teddy.1.mesh");
    teddy.SetShaderPath("assets/shaders/mesh.vs", "assets/shaders/mesh.fs");
    teddy.SetBoundingBoxSize(10.0);
    teddy.MoveTo(glm::vec3(10, 0, -20));
    teddy.Setup();
    teddy.EnableGUI(false);
    window.AddObject(&teddy);

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
        window.AddObject(&smileBoxs[i]);
    }

    double now;
    double lastUpdateTime = 0;
    double lastRenderTime = 0;
    while (window.ContinueLoop()) {
        lastRenderTime = now;
        lastUpdateTime = now;
        now = glfwGetTime();
        ////// Update Logic
        window.Update(now, lastUpdateTime);
        ////// Render Frame, you can choose to render only when 1 / (now - lastRenderTime) = 60Hz.
        window.Render(now, lastRenderTime);
        ////// Finish Render
        window.SwapBuffersAndPollEvents();
    }

    return 0;
}
