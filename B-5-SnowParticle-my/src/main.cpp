#if 1
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

#include "log.h"
#include "Scene.hpp"
#include "Window.hpp"
#include "GUI.hpp"
#include "Shader.hpp"
// #include "SkyBox.hpp"
// #include "Terrian.hpp"
#include "Snow.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

int main() {
    GLint success = 0;
    Window window(&success, "B-5-SnowParticle", 800, 600);
    if (success != 1) {
        log_fatal("Failed to create GLFW window");
        return -1;
    }

    Shader ourShader("assets/6.2.coordinate_systems.vs", "assets/6.2.coordinate_systems.fs");
    // ourShader.Setup();
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
    camera.setPosition(glm::vec3(0.0f, 10.0f, 200.0f));
    camera.setMaxRenderDistance(1e6f);

    Scene scene;
    // SkyBox skybox;
    // skybox.SetShaderPath("assets/skybox.vs", "assets/skybox.fs");
    // skybox.SetTopImagePath("assets/SkyBox/SkyBox4.bmp");
    // skybox.SetNorthImagePath("assets/SkyBox/SkyBox0.bmp");
    // skybox.SetEastImagePath("assets/SkyBox/SkyBox1.bmp", -90);
    // skybox.SetSouthImagePath("assets/SkyBox/SkyBox2.bmp");
    // skybox.SetWestImagePath("assets/SkyBox/SkyBox3.bmp", -90);
    // skybox.SetBottomImagePath("assets/SkyBox/SkyBox5.bmp");
    // skybox.SetBoxWidth(1e5f);
    // skybox.MoveWith(glm::vec3(0.0f, 1e5f/2, 0.0f));
    // skybox.Setup();

    // Terrian terrian;
    // terrian.SetShaderPath(
    //     "assets/8.3.gpuheight.vs", "assets/8.3.gpuheight.fs",
    //     "assets/8.3.gpuheight.tcs", "assets/8.3.gpuheight.tes"
    // );
    // // terrian.SetHeightMapPath("assets/textures/iceland_heightmap.png");
    // terrian.SetHeightMapPath("assets/TerrianHW/heightmap.bmp");
    // terrian.SetTexturePath("assets/TerrianHW/terrain-texture3.bmp");
    // terrian.MoveWith(glm::vec3(0.0f, -2.0f, 0.0f));
    // terrian.Setup();

    Snow::Snow snow;


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
    // gui.subscribe(&terrian);

    // window.AddObject(&terrian);

    double now;
    double lastUpdateTime = 0;
    double lastRenderTime = 0;
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

    // while (window.continueLoop()) {
	// 	glfwPollEvents();
	// 	// do_movement();
	// 	//render
	// 	glClearColor(0.0,0.0,0.0,1.0);
	// 	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	// 	glm::mat4 projection(1.0f);
	// 	glm::mat4 model(1.0f);
	// 	glm::mat4 view = camera.GetViewMatrix();
	// 	projection = glm::perspective(glm::radians(45.0f),screenWidth/screenHeight,0.1f,2000.f);
    //     {
    //         glm::mat4 model = glm::mat4(1.0f);
    //         model = glm::translate(model, position);
    //         floor.render(model,view,projection);
    //     }
	// 	snow.Render(deltaTime,model,view,projection, floor.y);

	// 	GLfloat currentFrame = glfwGetTime();
	// 	deltaTime = currentFrame - lastFrame;
	// 	lastFrame = currentFrame;
	// 	glfwSwapBuffers(window);
	// 	glfwPollEvents();
	// }
    while (window.continueLoop()) {
        lastRenderTime = now;
        lastUpdateTime = now;
        now = glfwGetTime();
        double deltaUpdateTime = now - lastUpdateTime;
        double deltaRenderTime = now - lastRenderTime;

        ////// logic update
        window.processInput(deltaUpdateTime, deltaRenderTime);
        // skybox.update(now, deltaUpdateTime);
        // terrian.update(now, deltaUpdateTime);
        // gui.update();

        ////// frame render
		glClearColor(0.0,0.0,0.0,1.0);
        // glClearColor(64 / 256., 48 / 256., 64 / 256., 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // ourShader.use();
        // float screenRatio = (float)window.SCR_WIDTH / (float)window.SCR_HEIGHT;
        // glm::mat4 projection = camera.GetProjectionMatrix(lastRenderTime, now, screenRatio);
        // glm::mat4 view = camera.GetViewMatrix();
        // ourShader.setMat4("projection", projection);
        // ourShader.setMat4("view", view);

        glBindVertexArray(VAO);
        for (int i = 0; i < cubePositions.size(); i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, (float)(45 + now * i), scene.Front());
            ourShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        {
            glm::mat4 projection(1.0f);
            glm::mat4 model(1.0f);
            glm::mat4 view = camera.GetViewMatrix();
            float screenRatio = (float)window.SCR_WIDTH / (float)window.SCR_HEIGHT;
            projection = glm::perspective(glm::radians(45.0f),screenRatio,0.1f,2000.f);
            {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, position);
                // floor.render(model,view,projection);
            }
            float deltaTime = deltaRenderTime;
            snow.Render(deltaTime,model,view,projection, -1.0);
        }

        // skybox.render(now, deltaRenderTime, view, projection);
        // terrian.render(now, deltaRenderTime, view, projection);
        // gui.render(window.w, lastRenderTime, now);

        window.swapBuffersAndPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    return 0;
}


#else


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "log.h"
// #include "GUI.hpp"
#include "Camera.hpp"
#include "Snow.hpp"
// #include "floor.h"

#include <iostream>

void key_callback(GLFWwindow* window,int key, int scancode,int action,int mode);
void do_movement();
void mouse_callback(GLFWwindow* window,double xpos,double ypos);
void scroll_callback(GLFWwindow* window,double xoffset,double yoffset);

GLfloat screenWidth = 1600;
GLfloat screenHeight = 1000;
Camera::Camera camera(glm::vec3(0.0f,0.0f,0.0f));
bool keys[1024];
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat lastX = 400,lastY = 300;
bool firstMouse = true;
string str_fps;
char c[8];
int FrameRate = 0;
int FrameCount = 0;
int timeLeft = 0;

int main(){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


	GLFWwindow* window = glfwCreateWindow(screenWidth,screenHeight,"Learn OpenGL",NULL,NULL);
	if(window == NULL){
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window,mouse_callback);
	glfwSetScrollCallback(window,scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // Floor floor;
	Snow::Snow snow;
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

	while(!glfwWindowShouldClose(window)){
		glfwPollEvents();
		do_movement();
		//render
		glClearColor(0.0,0.0,0.0,1.0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
		glm::mat4 projection(1.0f);
		glm::mat4 model(1.0f);
		glm::mat4 view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(45.0f),screenWidth/screenHeight,0.1f,2000.f);
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, position);
            // floor.render(model,view,projection);
        }
		snow.Render(deltaTime,model,view,projection, -1.0);

		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();

	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode){
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window,GL_TRUE);
	if(action == GLFW_PRESS)
		keys[key] = true;
	else if(action == GLFW_RELEASE)
		keys[key] = false;
}

void do_movement(){
	GLfloat cameraSpeed = 5.0f*deltaTime;
	if(keys[GLFW_KEY_W])
		camera.ProcessKeyboard(Camera::FORWARD,deltaTime);
	if(keys[GLFW_KEY_S])
		camera.ProcessKeyboard(Camera::BACKWARD,deltaTime);
	if(keys[GLFW_KEY_A])
		camera.ProcessKeyboard(Camera::LEFTS,deltaTime);
	if(keys[GLFW_KEY_D])
		camera.ProcessKeyboard(Camera::RIGHTS,deltaTime);
}

void mouse_callback(GLFWwindow* window,double xpos,double ypos){
	if(firstMouse){
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	camera.ProcessMouseMovement(xoffset,yoffset);
}

void scroll_callback(GLFWwindow* window,double xoffset,double yoffset){
	camera.ProcessMouseScroll(yoffset);
}


#endif