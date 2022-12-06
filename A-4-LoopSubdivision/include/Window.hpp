#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdint.h>

#include <string>
#include <map>
#include <vector>
#include <queue>
#include <limits>

#include "Object/BaseObject.hpp"
#include "log.h"
#include "Camera.hpp"

class Window {
public:
    enum class KeyState : bool
    {
        STATE_RELEASE, STATE_PRESS
    };

    uint32_t SCR_WIDTH  = 1600;
    uint32_t SCR_HEIGHT = 900;
    static inline std::string name = "Mayeths' OpenGL Program";
    GLFWwindow* w;
    Camera camera;
    bool firstMouse = true;
    bool keyTabStillPressing = false;
    bool inGodMod = false;
    float mouseLastX =  800.0f / 2.0;
    float mouseLastY =  600.0 / 2.0;

    bool enableInputListening = true;
    bool enableKeyListening = true;

    std::vector<KeyState> keystate1;
    std::vector<KeyState> keystate2;
    std::vector<KeyState> *keystate = &keystate1;
    std::vector<KeyState> *old_keystate = &keystate2;

public:
    // initializer
    Window(int *success, std::string name = "Mayeths' Terrain Engine", uint32_t scrW = 1600, uint32_t scrH = 900) : camera(Camera::INIT_WORLD_UP)
    {
        this->SCR_WIDTH  = scrW;
        this->SCR_HEIGHT = scrH;
        *success = 1;
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);                // comment this line in a release build!
        this->keystate1.resize(1024, KeyState::STATE_RELEASE);
        this->keystate2.resize(1024, KeyState::STATE_RELEASE);

        // glfwWindowHint(GLFW_SAMPLES, 4);
        // glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);

        // glfw window creation
        this->w = glfwCreateWindow(scrW, scrH, name.c_str(), NULL, NULL);
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
            Window::printInfomation();
            log_debug("GLFW window initialized");
        } else {
            log_fatal("Cannot initialize GLFW window");
            glfwTerminate();
            return;
        }

        glfwSetFramebufferSizeCallback(this->w, &Window::framebuffer_size_callback);
        glfwSetWindowFocusCallback(this->w, &Window::focus_callback);
        glfwSetCursorPosCallback(this->w, &Window::mouse_callback);
        glfwSetScrollCallback(this->w, &Window::scroll_callback);
        glfwSetInputMode(this->w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glEnable(GL_DEPTH_TEST);

        camera.setPosition(Camera::INIT_POSITION)
            .setLookAtTarget(Camera::INIT_FRONT)
        ;
    }

    ~Window()
    {
        this->terminate();
    }

    void processInput(float deltaUpdateTime, float deltaRenderTime)
    {
        bool keyTab = glfwGetKey(this->w, GLFW_KEY_TAB) == GLFW_PRESS;
        if (keyTab) {
            if (!this->keyTabStillPressing) {
                this->inGodMod = !this->inGodMod;
                int cursorMode = this->inGodMod ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
                glfwSetInputMode(this->w, GLFW_CURSOR, cursorMode);
                this->keyTabStillPressing = true;
            }
        } else {
            this->keyTabStillPressing = false;
        }
        if (this->inGodMod) {
            return;
        }

        bool keyW = glfwGetKey(this->w, GLFW_KEY_W) == GLFW_PRESS;
        bool keyS = glfwGetKey(this->w, GLFW_KEY_S) == GLFW_PRESS;
        bool keyA = glfwGetKey(this->w, GLFW_KEY_A) == GLFW_PRESS;
        bool keyD = glfwGetKey(this->w, GLFW_KEY_D) == GLFW_PRESS;
        bool keySpace = glfwGetKey(this->w, GLFW_KEY_SPACE) == GLFW_PRESS;
        bool keyLCtrl = glfwGetKey(this->w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
        bool keyRCtrl = glfwGetKey(this->w, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
        bool keyESC = glfwGetKey(this->w, GLFW_KEY_ESCAPE) == GLFW_PRESS;

        if (keyW) this->camera.ProcessKeyboard(FORWARD, deltaUpdateTime);
        if (keyS) this->camera.ProcessKeyboard(BACKWARD, deltaUpdateTime);
        if (keyA) this->camera.ProcessKeyboard(LEFT, deltaUpdateTime);
        if (keyD) this->camera.ProcessKeyboard(RIGHT, deltaUpdateTime);
        if (keySpace) this->camera.ProcessKeyboard(UP, deltaUpdateTime);
        if (keyLCtrl || keyRCtrl) this->camera.ProcessKeyboard(DOWN, deltaUpdateTime);

        if (keyESC) glfwSetWindowShouldClose(this->w, true);
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
        Window *self = (Window *)glfwGetWindowUserPointer(window);
        self->SCR_WIDTH = width;
        self->SCR_HEIGHT = height;
    }

    // glfw: whenever the mouse moves, this callback is called
    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
    {
        Window *self = (Window *)glfwGetWindowUserPointer(window);
        if (self->inGodMod)
            return;
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
    }

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        Window *self = (Window *)glfwGetWindowUserPointer(window);
        if (self->inGodMod)
            return;
        self->camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }

    static void focus_callback(GLFWwindow* window, int focused)
    {
        Window *self = (Window *)glfwGetWindowUserPointer(window);
        if (focused) {
            self->enableKeyListening = true;
        } else {
            std::fill((*self->keystate).begin(), (*self->keystate).end(), KeyState::STATE_RELEASE);
            self->enableKeyListening = false;
        }
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

        int major = 0, minor = 0, rev = 0;
        glfwGetVersion(&major, &minor, &rev);
        log_debug("GLFW version: %d.%d.%d", major, minor, rev);

        GLint maxTessLevel = 0;
        glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &maxTessLevel);
        log_debug("Max available tess level: %d", maxTessLevel);
    }

};
