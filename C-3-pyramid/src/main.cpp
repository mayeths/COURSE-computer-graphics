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

static int SCR_WIDTH  = 800;
static int SCR_HEIGHT = 600;




int main() {
    GLint success = 0;
    Window window(&success, "C-3-pyramid", 1600, 900);
    if (success != 1) {
        log_fatal("Failed to create GLFW window");
        return -1;
    }

    // Shader pyramidShader ("../src/materials.vs", "../src/materials.fs");
    // Shader lightCubeShader("../src/lightcube.vs", "../src/lightcube.fs");
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);"
        "}\n\0"
    ;
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (success == GL_TRUE) {
        log_debug("Vertex shader compile successfully");
    } else {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);
        log_fatal("Vertex shader compile failed: %s", infoLog);
        return -1;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "  FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\n\0"
    ;
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (success == GL_TRUE) {
        log_debug("Fragment shader compile successfully");
    } else {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), NULL, infoLog);
        log_fatal("Fragment shader compile failed: %s", infoLog);
        return -1;
    }
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(success == GL_TRUE) {
        log_debug("Shader program link successfully");
    } else {
        char infoLog[512];
        glGetShaderInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
        log_fatal("Shader program link failed: %s", infoLog);
        return -1;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLfloat vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f,
    };
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLAT), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    GUI gui(window);

    const double fpsLimit = 1.0 / 60.0;
    double lastUpdateTime = 0;
    double lastRenderTime = 0;
    while (window.continueLoop()) {
        double now = glfwGetTime();
        double deltaUpdateTime = now - lastUpdateTime;
        double deltaRenderTime = now - lastRenderTime;

        ////// logic update
        window.processInput(deltaUpdateTime);

        ////// frame render
        if ((now - lastRenderTime) >= fpsLimit) {
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);

            gui.update();
            gui.draw(deltaRenderTime);

            window.swapBuffersAndPollEvents();
            lastRenderTime = now;
        }

        lastUpdateTime = now;
    }

    return 0;
}
