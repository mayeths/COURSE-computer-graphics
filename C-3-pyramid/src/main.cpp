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

    Shader ourShader("assets/6.2.coordinate_systems.vs", "assets/6.2.coordinate_systems.fs");
    GLfloat vertices[] = {
        /** OpenGL is Right-handed system
         * By convention, OpenGL is a right-handed system.
         * What this basically says is that the positive x-axis is to your right, the 
         * positive y-axis is up and the positive z-axis is backwards. Think of your 
         * screen being the center of the 3 axes and the positive z-axis going through 
         * your screen towards you. The axes are drawn as follows:
         * https://learnopengl.com/Getting-started/Coordinate-Systems (#Going 3D)
         * To understand why it's called right-handed do the following:
         * Stretch your right-arm along the positive y-axis with your hand up top.
         * Let your thumb point to the right.
         * Let your pointing finger point up.
         * Now bend your middle finger downwards 90 degrees.
         * If you did things right, your thumb should point towards the positive x-axis, 
         * the pointing finger towards the positive y-axis and your middle finger towards 
         * the positive z-axis. If you were to do this with your left-arm you would see 
         * the z-axis is reversed. This is known as a left-handed system and is commonly 
         * used by DirectX. Note that in normalized device coordinates OpenGL actually 
         * uses a left-handed system (the projection matrix switches the handedness).
         */
        // positions         // texture coords
        // Back (z=-0.5 and xy plane same)
        -0.5f, -0.5f, -0.5f,  0.0f*2, 0.0f*2,
        +0.5f, -0.5f, -0.5f,  1.0f*2, 0.0f*2,
        +0.5f, +0.5f, -0.5f,  1.0f*2, 1.0f*2,
        +0.5f, +0.5f, -0.5f,  1.0f*2, 1.0f*2,
        -0.5f, +0.5f, -0.5f,  0.0f*2, 1.0f*2,
        -0.5f, -0.5f, -0.5f,  0.0f*2, 0.0f*2,
        // Front (z=0.5 and xy plane same)
        -0.5f, -0.5f,  0.5f,  0.0f*2, 0.0f*2,
        +0.5f, -0.5f,  0.5f,  1.0f*2, 0.0f*2,
        +0.5f, +0.5f,  0.5f,  1.0f*2, 1.0f*2,
        +0.5f, +0.5f,  0.5f,  1.0f*2, 1.0f*2,
        -0.5f, +0.5f,  0.5f,  0.0f*2, 1.0f*2,
        -0.5f, -0.5f,  0.5f,  0.0f*2, 0.0f*2,
        // Left (x=-0.5 and yz plane same)
        -0.5f, -0.5f, -0.5f,  0.0f*2, 0.0f*2,
        -0.5f, +0.5f, -0.5f,  1.0f*2, 0.0f*2,
        -0.5f, +0.5f, +0.5f,  1.0f*2, 1.0f*2,
        -0.5f, +0.5f, +0.5f,  1.0f*2, 1.0f*2,
        -0.5f, -0.5f, +0.5f,  0.0f*2, 1.0f*2,
        -0.5f, -0.5f, -0.5f,  0.0f*2, 0.0f*2,
        // Right (x=0.5 and yz plane same)
        +0.5f, -0.5f, -0.5f,  0.0f*2, 0.0f*2,
        +0.5f, +0.5f, -0.5f,  1.0f*2, 0.0f*2,
        +0.5f, +0.5f, +0.5f,  1.0f*2, 1.0f*2,
        +0.5f, +0.5f, +0.5f,  1.0f*2, 1.0f*2,
        +0.5f, -0.5f, +0.5f,  0.0f*2, 1.0f*2,
        +0.5f, -0.5f, -0.5f,  0.0f*2, 0.0f*2,
        // Bottom (y=-0.5 and xz plane same)
        -0.5f, -0.5f, -0.5f,  0.0f*2, 0.0f*2,
        +0.5f, -0.5f, -0.5f,  1.0f*2, 0.0f*2,
        +0.5f, -0.5f, +0.5f,  1.0f*2, 1.0f*2,
        +0.5f, -0.5f, +0.5f,  1.0f*2, 1.0f*2,
        -0.5f, -0.5f, +0.5f,  0.0f*2, 1.0f*2,
        -0.5f, -0.5f, -0.5f,  0.0f*2, 0.0f*2,
        // Top (y=0.5 and xz plane same)
        -0.5f, +0.5f, -0.5f,  0.0f*2, 0.0f*2,
        +0.5f, +0.5f, -0.5f,  1.0f*2, 0.0f*2,
        +0.5f, +0.5f, +0.5f,  1.0f*2, 1.0f*2,
        +0.5f, +0.5f, +0.5f,  1.0f*2, 1.0f*2,
        -0.5f, +0.5f, +0.5f,  0.0f*2, 1.0f*2,
        -0.5f, +0.5f, -0.5f,  0.0f*2, 0.0f*2,
    };
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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
    ourShader.setInt("texture1", 0);
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
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture2);

            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 view = glm::mat4(1.0f);
            glm::mat4 projection = glm::mat4(1.0f);
            model = glm::rotate(model, (float)now, glm::vec3(0.5f, 1.0f, 0.0f));
            view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
            projection = glm::perspective(glm::radians(45.0f), (float)window.SCR_WIDTH / (float)window.SCR_HEIGHT, 0.1f, 100.0f);

            ourShader.use();
            GLuint modelLoc = glGetUniformLocation(ourShader.ID, "model");
            GLuint viewLoc = glGetUniformLocation(ourShader.ID, "view");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
            ourShader.setMat4("projection", projection);

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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