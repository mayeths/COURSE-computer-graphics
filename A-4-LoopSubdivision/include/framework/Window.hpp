#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdint.h>
#include <stb_image.h>

#include <string>
#include <map>
#include <vector>
#include <queue>
#include <limits>
#include <set>

#include "Camera.hpp"
#include "GUI.hpp"
#include "Object.hpp"
#include "util/log.h"
#define MEMORY_IMPL
#include "util/memory.h"


class Window {
public:
    enum {
        STAT_FPS = 0x1, STAT_POSITION = 0x2, STAT_MEMORY = 0x4
    };
    int statistic = 0;

    uint32_t SCR_WIDTH  = 1600;
    uint32_t SCR_HEIGHT = 900;
    glm::vec4 backgroundColor = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f);
    static inline std::string name = "Mayeths' OpenGL Program";
    GLFWwindow* w;
    Camera camera;
    GUI gui;
    std::set<Object *> objects;
    float mouseLastX =  800.0f / 2.0;
    float mouseLastY =  600.0 / 2.0;
    bool firstMouse = true;
    bool enableKeyListening = true;
    bool keyTabStillPressing = false;
    bool inGodMod = false;

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
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);  // Enable OpenGL debug

        this->w = glfwCreateWindow(scrW, scrH, name.c_str(), NULL, NULL);
        if (!this->w) {
            log_fatal("Failed to create GLFW window");
            glfwTerminate();
            *success = 0;
            return;
        }
        glfwMakeContextCurrent(this->w);
        glfwSetWindowUserPointer(this->w, this);

        *success = glad_loader() && *success;
        if (!*success) {
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

        this->camera.SetPosition(Camera::INIT_POSITION);
        this->camera.SetLookAtTarget(Camera::INIT_FRONT);
        this->gui.Setup(this->w);
        this->gui.Subscribe(Window::gui_callback);

        Window::print_infomation();
        log_info("GLFW window initialized");
    }

    ~Window()
    {
        this->Terminate();
    }

    void Terminate()
    {
        glfwTerminate();
    }

    bool ContinueLoop()
    {
        return !glfwWindowShouldClose(this->w);
    }

    void EnableStatisticGUI(int flag)
    {
        this->statistic = flag;
    }

    void AddObject(Object *object)
    {
        this->objects.insert(object);
        GUIHandler *g = dynamic_cast<GUIHandler *>(object);
        if (g)
            this->gui.Subscribe(g);
    }

    void SubscribeGUI(GUI::Callback callback, void *data = nullptr)
    {
        this->gui.Subscribe(callback, data);
    }

    void Update(double now, double lastUpdateTime)
    {
        this->ProcessInput(now, lastUpdateTime);
        for (auto &object : this->objects) {
            object->Update(now, lastUpdateTime, this->w);
        }
    }

    void Render(double now, double lastRenderTime)
    {
        float bgr = this->backgroundColor.x;
        float bgg = this->backgroundColor.y;
        float bgb = this->backgroundColor.z;
        float bga = this->backgroundColor.w;
        glClearColor(bgr, bgg, bgb, bga);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float screenRatio = (float)this->SCR_WIDTH / (float)this->SCR_HEIGHT;
        glm::mat4 projection = camera.GetProjectionMatrix(now, lastRenderTime, screenRatio);
        glm::mat4 view = camera.GetViewMatrix();

        for (auto &object : this->objects) {
            object->Render(now, lastRenderTime, view, projection);
        }
        gui.Render(now, lastRenderTime);
        this->SwapBuffersAndPollEvents();
    }

    void ProcessInput(double now, double lastTime)
    {
        float deltaTime = now - lastTime;
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

        if (keyW) this->camera.ProcessKeyboard(FORWARD, deltaTime);
        if (keyS) this->camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (keyA) this->camera.ProcessKeyboard(LEFT, deltaTime);
        if (keyD) this->camera.ProcessKeyboard(RIGHT, deltaTime);
        if (keySpace) this->camera.ProcessKeyboard(UP, deltaTime);
        if (keyLCtrl || keyRCtrl) this->camera.ProcessKeyboard(DOWN, deltaTime);

        if (keyESC) glfwSetWindowShouldClose(this->w, true);
    }

    void SwapBuffersAndPollEvents()
    {
        glfwSwapBuffers(this->w);
        glfwPollEvents();
    }

    /***** utility functions *****/

    static int glad_loader()
    {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            log_fatal("Failed to initialize GLAD");
            return 0;
        }
        return 1;
    }

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
        Window *self = (Window *)glfwGetWindowUserPointer(window);
        self->SCR_WIDTH = width;
        self->SCR_HEIGHT = height;
    }

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
        self->enableKeyListening = (bool)focused;
    }

    static void gui_callback(double now, double lastTime, GLFWwindow *window, void *data)
    {
        Window *self = (Window *)glfwGetWindowUserPointer(window);

        ImGui::SetNextWindowPos(ImVec2(0, 60), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::Begin("Statistic", NULL,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings
        );
        ImGui::Text("Press Tab to enable/disable mouse");
        if (self->statistic & Window::STAT_FPS)
            ImGui::Text("FPS: %2.0f ", 1 / (now - lastTime));
        if (self->statistic & Window::STAT_MEMORY) {
            ImGui::SameLine();
            ImGui::Text("Memory: %s", readable_size(allocated_bytes).c_str());
        }
        if (self->statistic & Window::STAT_POSITION) {
            glm::vec3 pos = self->camera.Position;
            ImGui::Text("Camera Position: %2.0f %2.0f %2.0f", pos[0], pos[1], pos[2]);
        }
        ImVec2 window1_size = ImGui::GetWindowSize();
        ImGui::End();
    }

    static void print_infomation()
    {
        GLint GLmajor = 0, GLminor = 0, GLrev = 0, flags = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &GLmajor);
        glGetIntegerv(GL_MINOR_VERSION, &GLminor);
        glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &GLrev);
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        log_info("OpenGL version: %d.%d.%d", GLmajor, GLminor, GLrev);

        int major = 0, minor = 0, rev = 0;
        glfwGetVersion(&major, &minor, &rev);
        log_info("GLFW version: %d.%d.%d", major, minor, rev);

        GLint maxTessLevel = 0;
        glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &maxTessLevel);
        log_info("Max available tess level: %d", maxTessLevel);
    }

};
