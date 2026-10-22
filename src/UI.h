#pragma once

#include "OpenGLIncludes.h"
#include "Mesh.h"

struct Context;

class UI {
public:
    UI() {
        ImGui::CreateContext();
        io = ImGui::GetIO();
    }

    void setupImGuiStyle();

    void start();

    void update();

    void render();

    void stop();
private:
    void sceneWindow(Context& context);
    void addRemoveControls(Context& context);
    void materialEditor(Context& context, Material& material);

    ImGuiIO io;

    char modelPath[256] = "../assets/diamond.fbx";

    // Euler angles (degrees) shown for the selected mesh. Kept here, because recomputing them from the quaternion
    // every frame makes the values jump while dragging.
    glm::vec3 eulerDegrees{0.f};
    const void* eulerOwner = nullptr;

    std::string vendor;
    std::string version;
    std::string renderer;
};
