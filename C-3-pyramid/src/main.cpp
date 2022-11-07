#include <GLFW/glfw3.h>
#include <SOIL.h>
#include <glad/glad.h>
#include <stdio.h>

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "log.h"
#include "Window.hpp"
#include "GUI.hpp"
#include "Shader.hpp"

static int SCR_WIDTH  = 800;
static int SCR_HEIGHT = 600;




int main() {
    GLint success = 0;
    Window window(&success, "C-3-pyramid", 800, 600);
    if (success != 1) {
        log_fatal("Failed to create GLFW window");
        return -1;
    }

    Shader ourShader("assets/3.3.shader.vs", "assets/3.3.shader.fs");
    GLfloat vertices[] = {
        // positions         // colors
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top 
    };
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    GUI gui(window);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    const double fpsLimit = 1.0 / 60.0;
    double lastUpdateTime = 0;
    double lastRenderTime = 0;
    while (window.continueLoop()) {
        double now = glfwGetTime();
        double deltaUpdateTime = now - lastUpdateTime;
        double deltaRenderTime = now - lastRenderTime;

        ////// logic update
        window.processInput(deltaUpdateTime, deltaRenderTime);

        ////// frame render
        if ((now - lastRenderTime) >= fpsLimit) {
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            ourShader.use();
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);

            gui.update();
            gui.draw(deltaRenderTime);

            window.swapBuffersAndPollEvents();
            lastRenderTime = now;
        }

        lastUpdateTime = now;
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    return 0;
}
