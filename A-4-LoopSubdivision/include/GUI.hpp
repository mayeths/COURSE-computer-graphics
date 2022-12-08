#pragma once
#include <list>
#include <vector>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Object/GUIHandlerObject.hpp"

#include "Window.hpp"

class GUI
{
public:
    using Callback = std::function<void(double now, double lastTime, GLFWwindow* w)>;
    GLFWwindow *window;
    std::vector<GUIHandlerObject *> objects;
    std::vector<GUI::Callback> functions;

    GUI(GLFWwindow *window, const char *GLSLVersion = "#version 330")
    {
        this->window = window;
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(this->window, true);
        ImGui_ImplOpenGL3_Init(GLSLVersion);
    }

    ~GUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void subscribe(GUIHandlerObject *obj)
    {
        this->objects.push_back(obj);
    }
    void subscribe(GUI::Callback callback)
    {
        this->functions.push_back(callback);
    }

    void refresh(double now, double lastRenderTime, GLFWwindow *window)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        for (int i = 0; i < objects.size(); i++)
            this->objects[i]->refresh(now, lastRenderTime);
        for (int i = 0; i < this->functions.size(); i++)
            this->functions[i](now, lastRenderTime, window);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
};
