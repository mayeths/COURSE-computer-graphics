#pragma once
#include <list>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Window.hpp"

class GUI
{
public:
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

    void update()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    void draw(double delta)
    {
        double xpos = ImGui::GetIO().DisplaySize.x - 64;
        double ypos = 32.0f;
        ImGui::SetNextWindowPos(ImVec2(xpos, ypos), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        ImGui::Begin("Stat", NULL, ImGuiWindowFlags_NoResize);
        // double framerate = ImGui::GetIO().Framerate;
        // double delta = 1000.0f / ImGui::GetIO().Framerate;
        double framerate = 1 / delta;
        ImGui::Text("%.0f FPS", framerate);
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
};
