#pragma once

#include "OpenGLIncludes.h"

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
    ImGuiIO io;

    std::string vendor;
    std::string version;
    std::string renderer;
};
