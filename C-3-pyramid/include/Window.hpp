#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdint.h>

#include <string>
#include <map>

#include "log.h"
#include "Camera.hpp"

class Window {
public:

    static inline uint32_t SCR_WIDTH  = 1600;
    static inline uint32_t SCR_HEIGHT = 900;
    static inline std::string name = "Mayeths' OpenGL Program";
    GLFWwindow* w;
    Camera camera;
    bool firstMouse = true;
    // float yaw   = -90.0f;
    // float pitch =  0.0f;
    float mouseLastX =  800.0f / 2.0;
    float mouseLastY =  600.0 / 2.0;
    // float mouseSensitivity = 0.05f;
    // float cameraSpeedScale = 5.0f;
    // double lastScrollPollTime = -100000.0f;
    // double lastScrollPollYOffset = 0.0f;
    // const glm::vec3 worldNormal = glm::vec3(0.0f, 1.0f, 0.0f); // 法线（定义世界的上）
    // glm::vec3 cameraPos;
    // glm::vec3 cameraFront;
    // glm::vec3 cameraUp;
    // glm::vec3 direction;
    // glm::mat4 view;

    // struct KeyCallbackParam {
    //     Window *window;
    //     int type;
    //     double deltaUpdateTime;
    //     double deltaRenderTime;
    // };
    // typedef void (*KeyCallback)(GLFWwindow* window, double xpos, double ypos, float updateTime, float renderTime);
    // std::map<int, KeyCallback> inputCallback;

public:
    // initializer
    Window(int *success, std::string name = "Mayeths' Terrain Engine", uint32_t scrW = 1600, uint32_t scrH = 900) : camera(Camera::INIT_WORLD_UP)
    {
        Window::SCR_WIDTH  = scrW;
        Window::SCR_HEIGHT = scrH;
        *success = 1;
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);                // comment this line in a release build!

        // glfwWindowHint(GLFW_SAMPLES, 4);
        // glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);

        // glfw window creation
        this->w = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, name.c_str(), NULL, NULL);
        if (!this->w) {
            log_fatal("Failed to create GLFW window");
            glfwTerminate();
            *success = 0;
            return;
        }
        glfwMakeContextCurrent(this->w);
        glfwSetWindowUserPointer(this->w, this);

        *success = gladLoader() && *success;
        if (*success) {
            log_debug("GLFW window correctly initialized");
        }
        Window::printInfomation();

        glfwSetFramebufferSizeCallback(this->w, &Window::framebuffer_size_callback);
        glfwSetCursorPosCallback(this->w, &Window::mouse_callback);
        glfwSetScrollCallback(this->w, &Window::scroll_callback);
        glfwSetInputMode(this->w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        // glfwSetInputMode(this->w, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        // glfwSetWindowUserPointer(this->w, this);
        glEnable(GL_DEPTH_TEST);

        camera.setPosition(Camera::INIT_POSITION)
            .setLookAtTarget(Camera::INIT_FRONT)
            .setMovementSpeed(Camera::INIT_MOVEMENT_SPEED)
            .setMouseSensitivity(Camera::INIT_MOUSE_SENSITIVITY)
        ;
    }

    ~Window()
    {
        this->terminate();
    }

    void processInput(float deltaUpdateTime, float deltaRenderTime)
    {
        if (glfwGetKey(this->w, GLFW_KEY_W) == GLFW_PRESS)
            this->camera.ProcessKeyboard(FORWARD, deltaUpdateTime);
        if (glfwGetKey(this->w, GLFW_KEY_S) == GLFW_PRESS)
            this->camera.ProcessKeyboard(BACKWARD, deltaUpdateTime);
        if (glfwGetKey(this->w, GLFW_KEY_A) == GLFW_PRESS)
            this->camera.ProcessKeyboard(LEFT, deltaUpdateTime);
        if (glfwGetKey(this->w, GLFW_KEY_D) == GLFW_PRESS)
            this->camera.ProcessKeyboard(RIGHT, deltaUpdateTime);
        if (glfwGetKey(this->w, GLFW_KEY_SPACE) == GLFW_PRESS)
            this->camera.ProcessKeyboard(UP, deltaUpdateTime);
        if (glfwGetKey(this->w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
            glfwGetKey(this->w, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
            this->camera.ProcessKeyboard(DOWN, deltaUpdateTime);

        if (glfwGetKey(this->w, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(this->w, true);

        // const float cameraSpeed = cameraSpeedScale * deltaUpdateTime;
        // if (glfwGetKey(this->w, GLFW_KEY_W) == GLFW_PRESS)
        //     cameraPos += glm::normalize(cameraFront) * cameraSpeed;
        // if (glfwGetKey(this->w, GLFW_KEY_S) == GLFW_PRESS)
        //     cameraPos -= glm::normalize(cameraFront) * cameraSpeed;
        // if (glfwGetKey(this->w, GLFW_KEY_A) == GLFW_PRESS)
        //     cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        // if (glfwGetKey(this->w, GLFW_KEY_D) == GLFW_PRESS)
        //     cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }

    int gladLoader()
    {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            log_fatal("Failed to initialize GLAD");
            return 0;
        }
        return 1;
    }

    // screen settings
    void terminate()
    {
        glfwTerminate();
    }

    bool continueLoop() {
        return !glfwWindowShouldClose(this->w);
    }

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    // glfw: whenever the mouse moves, this callback is called
    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
    {
        Window *self = (Window *)glfwGetWindowUserPointer(window);
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (self->firstMouse) {
            self->mouseLastX = xpos;
            self->mouseLastY = ypos;
            self->firstMouse = false;
        }

        float xoffset = xpos - self->mouseLastX;
        float yoffset = self->mouseLastY - ypos;
        self->mouseLastX = xpos;
        self->mouseLastY = ypos;

        self->camera.ProcessMouseMovement(xoffset, yoffset);

        // xoffset *= self->mouseSensitivity;
        // yoffset *= self->mouseSensitivity;

        // self->yaw += xoffset;
        // self->pitch += yoffset;

        // if (self->pitch > 89.0f)
        //     self->pitch = 89.0f;
        // if (self->pitch < -89.0f)
        //     self->pitch = -89.0f;

        // glm::vec3 front;
        // front.x = cos(glm::radians(self->yaw)) * cos(glm::radians(self->pitch));
        // front.y = sin(glm::radians(self->pitch));
        // front.z = sin(glm::radians(self->yaw)) * cos(glm::radians(self->pitch));
        // self->cameraFront = glm::normalize(front);
    }

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        Window *self = (Window *)glfwGetWindowUserPointer(window);
        self->camera.ProcessMouseScroll(static_cast<float>(yoffset));
        // double maximumYOffset = 5.0f;
        // self->lastScrollPollYOffset = yoffset / maximumYOffset; /* G102 is 3.0f */
        // self->lastScrollPollTime = glfwGetTime();
    }

    // put this at the end of the main
    void swapBuffersAndPollEvents() {
        glfwSwapBuffers(this->w);
        glfwPollEvents();
    }

    static void printInfomation()
    {
        GLint GLmajor = 0, GLminor = 0, GLrev = 0, flags = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &GLmajor);
        glGetIntegerv(GL_MINOR_VERSION, &GLminor);
        glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &GLrev);
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        log_debug("OpenGL version: %d.%d.%d", GLmajor, GLminor, GLrev);

        int major, minor, rev;
        glfwGetVersion(&major, &minor, &rev);
        log_debug("GLFW version: %d.%d.%d", major, minor, rev);
    }

};
