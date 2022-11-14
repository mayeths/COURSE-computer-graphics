#pragma once
#include <list>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Window.hpp"
#include "drawableObject.hpp"
#include "utils.hpp"


class GUI : public drawableObject
{
public:
    GUI(Window& w);
    ~GUI();

    bool ensureEnvironment();

    virtual void draw();
    virtual void update();

    GUI& subscribe(drawableObject* subscriber);

private:
    // the other drawableObjects expose their setGui() methods (because he let the user handle their own attributes), so they can be subscribed to the GUI class
    // the GUI class will call the setGui() method for each subscriber
    std::list<drawableObject*> subscribers;

};

bool GUI::ensureEnvironment()
{
    return GUI::scene != nullptr;
}

GUI::GUI(Window& window)
{
    // GUI
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
    const char* glsl_version = "#version 150";
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void GUI::draw()
{
    if (!this->ensureEnvironment()) {
        return;
    }
    sceneElements& scene = *GUI::scene;

    for (drawableObject* obj : subscribers) {
        obj->setGui();
    }

    ImGui::Begin("Scene controls: ");
//     ImGui::TextColored(ImVec4(1, 1, 0, 1), "Other controls");
//     if (ImGui::DragFloat3("Light Position", &scene.lightDir[0], 0.01, -1.0, 1.0)) {
//         auto saturate = [](float v) { return std::min(std::max(v, 0.0f), 0.8f); };
//         scene.lightDir.y = saturate(scene.lightDir.y);
//         //skybox.update();
//     }
    // ImGui::InputFloat3("Camera Position", &(scene.cam->Position[0]));
//     ImGui::ColorEdit3("Light color", (float*)&scene.lightColor);
//     ImGui::ColorEdit3("Fog color", (float*)&scene.fogColor);
//     ImGui::SliderFloat("Camera speed", &scene.cam->MovementSpeed, 0.0, SPEED*3.0);




//     ImGui::Checkbox("Wireframe mode", &scene.wireframe);

//     if (ImGui::Button("Generate seed"))
//         scene.seed = genRandomVec3();
//     //ImGui::SameLine();
//     //ImGui::Text("Generate a new seed");
//     ImGui::SameLine();
//     if (ImGui::Button("Use default seed"))
//         scene.seed = glm::vec3(0.0, 0.0, 0.0);

//     /*ImGui::SameLine();
//     if (ImGui::Button("Default Preset")) {
//         volumetricClouds.DefaultPreset();
//         lightDir.y = 0.5;
//     }*/
//     //ImGui::SameLine();
//     /*if (ImGui::Button("Sunset Preset 1")) {
//         skybox.SunsetPreset();
//     }
//     ImGui::SameLine();
//     if (ImGui::Button("Sunset Preset 2")) {
//         skybox.SunsetPreset1();
//     }*/

    double framerate = ImGui::GetIO().Framerate;
    double delta = 1000.0f / ImGui::GetIO().Framerate;
    ImGui::Text("%.0f FPS", framerate);
    ImGui::End();
        

    // //actual drawing
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::update()
{
    if (!this->ensureEnvironment()) {
        return;
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

GUI & GUI::subscribe(drawableObject * subscriber)
{
    subscribers.push_back(subscriber);
    return *this;
}


GUI::~GUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

