#include <GLFW/glfw3.h>
#include <SOIL.h>
#include <glad/glad.h>
#include <stdio.h>

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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

    Shader ourShader("assets/4.2.shader.vs", "assets/4.2.shader.fs");
    GLfloat vertices[] = {
        // positions         // colors         // texture coords
         0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f*2, 1.0f*2, // ↗
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f*2, 0.0f*2, // ↘
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f*2, 0.0f*2, // ↙
        -0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 0.0f, 0.0f*2, 1.0f*2  // ↖
    };
    GLuint indices[] = {
        0, 1, 3,
        1, 2, 3,
    };
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    GLuint texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    /* http://www.c-jump.com/bcc/common/Talk3/OpenGL/Wk07_texture/W01_0190_minification_and_magn.htm
        Ideally, one screen pixel maps to one of the texels in the texture
        1. (magnification) As the object gets closer to the viewer,
        multiple screen pixels can map to a single texel
        Also, what should happen if one screen pixel ends up sampling multiple texels?
        2. (minification) If the object moves farther away,
        many texels of the image map to a single pixel on the screen
        What happens when part of the texture is sampled by a single pixel?
    */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    int width, height, nrChannels;
    GLubyte *data;
    stbi_set_flip_vertically_on_load(true);
    data = stbi_load("assets/textures/container.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        log_error("Failed to load texture");
    }
    stbi_image_free(data);

    GLuint texture2;
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    data = stbi_load("assets/textures/awesomeface.png", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        log_error("Failed to load texture");
    }
    stbi_image_free(data);

    ourShader.use();
    glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
    ourShader.setInt("texture2", 1);

    GUI gui(window);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    const double fpsLimit = 60;
    double lastUpdateTime = 0;
    double lastRenderTime = 0;
    while (window.continueLoop()) {
        double now = glfwGetTime();
        double deltaUpdateTime = now - lastUpdateTime;
        double deltaRenderTime = now - lastRenderTime;

        ////// logic update
        window.processInput(deltaUpdateTime, deltaRenderTime);

        ////// frame render
        if ((now - lastRenderTime) >= 1.0 / fpsLimit) {
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture2);

            ourShader.use();
            glBindVertexArray(VAO);
            // glDrawArrays(GL_TRIANGLES, 0, 3);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
