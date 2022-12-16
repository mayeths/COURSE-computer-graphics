#pragma once
#include <list>
#include <vector>
#include <utility>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <util/raii.hpp>


class GUIHandler
{
public:
    virtual void RenderGUI(double now, double lastTime, GLFWwindow *window) = 0;
};


class GUI
{
public:
    using Callback = std::function<void(double now, double lastTime, GLFWwindow* w, void *data)>;
    GLFWwindow * window = nullptr;
    guard_t guard;
    std::vector<GUIHandler *> objects;
    std::vector<std::pair<GUI::Callback, void *>> functions;

    ~GUI()
    {
        if (this->guard.get()) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
    }

    void Setup(GLFWwindow *window, const char *GLSLVersion = "#version 330")
    {
        if (window == nullptr)
            return;
        this->window = window;
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(this->window, true);
        ImGui_ImplOpenGL3_Init(GLSLVersion);
        this->guard.set();
    }

    void Subscribe(GUIHandler *obj)
    {
        this->guard.ensure();
        this->objects.push_back(obj);
    }
    void Subscribe(GUI::Callback callback, void *data = nullptr)
    {
        this->guard.ensure();
        this->functions.push_back(std::make_pair(callback, data));
    }

    void Render(double now, double lastRenderTime)
    {
        this->guard.ensure();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        for (int i = 0; i < this->objects.size(); i++)
            this->objects[i]->RenderGUI(now, lastRenderTime, window);
        for (int i = 0; i < this->functions.size(); i++) {
            GUI::Callback callback = this->functions[i].first;
            void *data = this->functions[i].second;
            callback(now, lastRenderTime, this->window, data);
        }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
};
