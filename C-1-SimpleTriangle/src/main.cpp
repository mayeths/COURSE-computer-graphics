#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.hpp"
#include "log.h"
#include "GUI.hpp"

#include <iostream>

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool openTab = true;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SimpleTriangle", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader shader("./assets/shader.vs", "./assets/shader.fs");
    shader.Setup();

    float vertices[] = {
        // positions
         0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f,
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    GUI gui(window);
    gui.subscribe([&](GLFWwindow *w, double lastRenderTime, double now) {
        ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH-60, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::Begin("Stats", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        ImGui::SameLine();
        ImGui::Text("%.0f FPS", 1 / (now - lastRenderTime));
        ImGui::End();
    });
    gui.subscribe([&](GLFWwindow *w, double lastRenderTime, double now) {
        ImGui::SetNextWindowPos(ImVec2(0, 30), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::Begin("Tips", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        ImGui::Text("Right click to open/close color picker");
        ImGui::End();
    });

    ImVec4 rgb(0.23f, 1.0f, 1.0f, 1.0f);

    gui.subscribe([&](GLFWwindow *w, double lastRenderTime, double now) {
        if (openTab) {
            ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::Begin("Color Picker");
            ImGui::Spacing();
            ImGui::ColorEdit4("RGB", (float*)&rgb, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float);
            ImGui::DragFloat4("Raw RGB values", (float*)&rgb, 0.23f, 1.0f, 1.0f);
            glm::vec4 color(rgb.x, rgb.y, rgb.z, rgb.w);
            shader.setVec4("color", color);
            ImGui::End();
        }
    });

    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    float screenRatio = (float)SCR_WIDTH / (float)SCR_HEIGHT;

    double now;
    double lastUpdateTime = 0;
    double lastRenderTime = 0;
    while (!glfwWindowShouldClose(window)) {
        lastRenderTime = now;
        lastUpdateTime = now;
        now = glfwGetTime();
        double deltaUpdateTime = now - lastUpdateTime;
        double deltaRenderTime = now - lastRenderTime;

        // logic
        processInput(window);
        gui.update();
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, position);
            model = glm::rotate(model, (float)(now), glm::vec3(0.0f, 0.0f, -1.0f));
            shader.setMat4("model", model);
        }
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), screenRatio, 0.1f, 100.0f);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        //render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        shader.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        gui.render(window, lastRenderTime, now);

        // misc
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        openTab = !openTab;
    }
}
