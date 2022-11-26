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
#include "Terrian.hpp"

int main() {
    GLint success = 0;
    Window window(&success, "C-3-pyramid", 800, 600);
    if (success != 1) {
        log_fatal("Failed to create GLFW window");
        return -1;
    }

    Shader ourShader("assets/6.2.coordinate_systems.vs", "assets/6.2.coordinate_systems.fs");
    ourShader.Setup();
#define scale 1.0f
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
        -0.5f * scale, -0.5f * scale, -0.5f * scale,  0.0f*2, 0.0f*2,
        +0.5f * scale, -0.5f * scale, -0.5f * scale,  1.0f*2, 0.0f*2,
        +0.5f * scale, +0.5f * scale, -0.5f * scale,  1.0f*2, 1.0f*2,
        +0.5f * scale, +0.5f * scale, -0.5f * scale,  1.0f*2, 1.0f*2,
        -0.5f * scale, +0.5f * scale, -0.5f * scale,  0.0f*2, 1.0f*2,
        -0.5f * scale, -0.5f * scale, -0.5f * scale,  0.0f*2, 0.0f*2,
        // Front (z=0.5 and xy plane same)
        -0.5f * scale, -0.5f * scale,  0.5f * scale,  0.0f*2, 0.0f*2,
        +0.5f * scale, -0.5f * scale,  0.5f * scale,  1.0f*2, 0.0f*2,
        +0.5f * scale, +0.5f * scale,  0.5f * scale,  1.0f*2, 1.0f*2,
        +0.5f * scale, +0.5f * scale,  0.5f * scale,  1.0f*2, 1.0f*2,
        -0.5f * scale, +0.5f * scale,  0.5f * scale,  0.0f*2, 1.0f*2,
        -0.5f * scale, -0.5f * scale,  0.5f * scale,  0.0f*2, 0.0f*2,
        // Left (x=-0.5 and yz plane same)
        -0.5f * scale, -0.5f * scale, -0.5f * scale,  0.0f*2, 0.0f*2,
        -0.5f * scale, +0.5f * scale, -0.5f * scale,  1.0f*2, 0.0f*2,
        -0.5f * scale, +0.5f * scale, +0.5f * scale,  1.0f*2, 1.0f*2,
        -0.5f * scale, +0.5f * scale, +0.5f * scale,  1.0f*2, 1.0f*2,
        -0.5f * scale, -0.5f * scale, +0.5f * scale,  0.0f*2, 1.0f*2,
        -0.5f * scale, -0.5f * scale, -0.5f * scale,  0.0f*2, 0.0f*2,
        // Right (x=0.5 and yz plane same)
        +0.5f * scale, -0.5f * scale, -0.5f * scale,  0.0f*2, 0.0f*2,
        +0.5f * scale, +0.5f * scale, -0.5f * scale,  1.0f*2, 0.0f*2,
        +0.5f * scale, +0.5f * scale, +0.5f * scale,  1.0f*2, 1.0f*2,
        +0.5f * scale, +0.5f * scale, +0.5f * scale,  1.0f*2, 1.0f*2,
        +0.5f * scale, -0.5f * scale, +0.5f * scale,  0.0f*2, 1.0f*2,
        +0.5f * scale, -0.5f * scale, -0.5f * scale,  0.0f*2, 0.0f*2,
        // Bottom (y=-0.5 and xz plane same)
        -0.5f * scale, -0.5f * scale, -0.5f * scale,  0.0f*2, 0.0f*2,
        +0.5f * scale, -0.5f * scale, -0.5f * scale,  1.0f*2, 0.0f*2,
        +0.5f * scale, -0.5f * scale, +0.5f * scale,  1.0f*2, 1.0f*2,
        +0.5f * scale, -0.5f * scale, +0.5f * scale,  1.0f*2, 1.0f*2,
        -0.5f * scale, -0.5f * scale, +0.5f * scale,  0.0f*2, 1.0f*2,
        -0.5f * scale, -0.5f * scale, -0.5f * scale,  0.0f*2, 0.0f*2,
        // Top (y=0.5 and xz plane same)
        -0.5f * scale, +0.5f * scale, -0.5f * scale,  0.0f*2, 0.0f*2,
        +0.5f * scale, +0.5f * scale, -0.5f * scale,  1.0f*2, 0.0f*2,
        +0.5f * scale, +0.5f * scale, +0.5f * scale,  1.0f*2, 1.0f*2,
        +0.5f * scale, +0.5f * scale, +0.5f * scale,  1.0f*2, 1.0f*2,
        -0.5f * scale, +0.5f * scale, +0.5f * scale,  0.0f*2, 1.0f*2,
        -0.5f * scale, +0.5f * scale, -0.5f * scale,  0.0f*2, 0.0f*2,
    };
    const std::vector<glm::vec3> cubePositions {
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

    int width, height, nchannels;
    GLubyte *data;
    GLenum format;
    stbi_set_flip_vertically_on_load(true);
    data = stbi_load("assets/textures/container.jpg", &width, &height, &nchannels, 0);
    format = nchannels == 4 ? GL_RGBA : GL_RGB;
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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
    data = stbi_load("assets/textures/awesomeface.png", &width, &height, &nchannels, 0);
    format = nchannels == 4 ? GL_RGBA : GL_RGB;
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        log_error("Failed to load texture");
    }
    stbi_image_free(data);

    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);

    Camera &camera = window.camera;
    camera.setPosition(glm::vec3(0.0f, 3.0f, 3.0f));
    camera.setMaxRenderDistance(1e6f);

    Scene scene;
    SkyBox skybox;
    skybox.SetShaderPath("assets/skybox.vs", "assets/skybox.fs");
    skybox.SetTopImagePath("assets/SkyBox/SkyBox4.bmp");
    skybox.SetNorthImagePath("assets/SkyBox/SkyBox0.bmp");
    skybox.SetEastImagePath("assets/SkyBox/SkyBox1.bmp", -90);
    skybox.SetSouthImagePath("assets/SkyBox/SkyBox2.bmp");
    skybox.SetWestImagePath("assets/SkyBox/SkyBox3.bmp", -90);
    skybox.SetBottomImagePath("assets/SkyBox/SkyBox5.bmp");
    skybox.SetBoxWidth(1e5f);
    skybox.MoveWith(glm::vec3(0.0f, 1e5f/2, 0.0f));
    skybox.Setup();

    Terrian terrian;
    terrian.SetShaderPath(
        "assets/8.3.gpuheight.vs", "assets/8.3.gpuheight.fs",
        "assets/8.3.gpuheight.tcs", "assets/8.3.gpuheight.tes"
    );
    // terrian.SetHeightMapPath("assets/textures/iceland_heightmap.png");
    terrian.SetHeightMapPath("assets/TerrianHW/heightmap.bmp");
    terrian.SetTexturePath("assets/TerrianHW/terrain-texture3.bmp");
    terrian.MoveWith(glm::vec3(0.0f, -3.0f, 0.0f));
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
        ImGui::Text("Press Tab to enter control mod");
        ImGui::End();
    });
    gui.subscribe(&terrian);

    window.AddObject(&terrian);

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
        glClearColor(64 / 256., 48 / 256., 64 / 256., 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        ourShader.use();
        float screenRatio = (float)window.SCR_WIDTH / (float)window.SCR_HEIGHT;
        glm::mat4 projection = camera.GetProjectionMatrix(lastRenderTime, now, screenRatio);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        glBindVertexArray(VAO);
        for (int i = 0; i < cubePositions.size(); i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, (float)(45 + now * i), scene.Front());
            ourShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        skybox.render(now, deltaRenderTime, view, projection);
        terrian.render(now, deltaRenderTime, view, projection);
        gui.render(window.w, lastRenderTime, now);

        window.swapBuffersAndPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    return 0;
}
