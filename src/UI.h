#pragma once

#include "OpenGLIncludes.h"

class UI {
public:
    UI() {
        ImGui::CreateContext();
        io = ImGui::GetIO();
    }

    void start();

    void update();

    void render();

    void stop();
private:
    ImGuiIO io;
};
