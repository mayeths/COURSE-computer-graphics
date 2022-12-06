#pragma once
#include <list>
#include <vector>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Object/DrawableObject.hpp"

#include "Window.hpp"

class GUI
{
public:
    using Callback = std::function<void(GLFWwindow* w, double lastRenderTime, double now)>;
    std::vector<DrawableObject *> subs;
    std::vector<GUI::Callback> callbacks;

    GUI(Window& window)
    {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window.w, true);
        const char* glsl_version = "#version 330";
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    ~GUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void subscribe(DrawableObject *obj)
    {
        this->subs.push_back(obj);
    }
    void subscribe(GUI::Callback callback)
    {
        this->callbacks.push_back(callback);
    }

    void update()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    void render(GLFWwindow *window, double lastRenderTime, double now)
    {
        double deltaRenderTime = now - lastRenderTime;
        for (int i = 0; i < subs.size(); i++) {
            subs[i]->GUIcallback(lastRenderTime, now);
        }
        for (int i = 0; i < this->callbacks.size(); i++) {
            this->callbacks[i](window, lastRenderTime, now);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
};
