#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdint.h>

#include <string>

#include "log.h"

class Window {
public:

    static inline uint32_t SCR_WIDTH  = 1600;
    static inline uint32_t SCR_HEIGHT = 900;
    static inline std::string name = "Mayeths' OpenGL Program";
    GLFWwindow* w;

public:
    // initializer
    Window(int *success, std::string name = "Mayeths' Terrain Engine", uint32_t scrW = 1600, uint32_t scrH = 900)
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

        *success = gladLoader() && *success;
        if (*success) {
            log_debug("GLFW window correctly initialized");
        }
        Window::printInfomation();

        glfwSetFramebufferSizeCallback(this->w, &Window::framebuffer_size_callback);
        glfwSetCursorPosCallback(this->w, &Window::mouse_callback);
        glfwSetScrollCallback(this->w, &Window::scroll_callback);
        // glfwSetInputMode(this->w, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        // glEnable(GL_DEPTH_TEST);
    }

    ~Window()
    {
        this->terminate();
    }

    void processInput(float updateTime, float renderTime)
    {
        if (glfwGetKey(this->w, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(this->w, true);
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
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
    {
    }

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
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
