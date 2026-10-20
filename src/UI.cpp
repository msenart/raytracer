#include "UI.h"
#include "Globals.h"
#include "Context.h"
void UI::start() {
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init("#version 300 es");
}

void UI::update() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    auto* context = static_cast<Context*>(glfwGetWindowUserPointer(window));
    auto& p = context->screen.raytracer_params;
    {
        bool changed = false;
        const unsigned int one = 1, max_iter_cap = 500, max_ms = 16, max_rs = 16;

        ImGui::Begin("Raytracer");

        ImGui::SeparatorText("Rendu");
        changed |= ImGui::SliderScalar("Max iter", ImGuiDataType_U32, &p.max_iter, &one, &max_iter_cap);
        changed |= ImGui::SliderFloat("Max distance", &p.max_distance, 1.f, 10000.f, "%.0f", ImGuiSliderFlags_Logarithmic);
        changed |= ImGui::SliderFloat("Background", &p.background_intensity, 0.f, 5.f);
        changed |= ImGui::SliderFloat("Epsilon", &p.eps, 1e-7f, 1e-1f, "%.7f", ImGuiSliderFlags_Logarithmic);

        ImGui::SeparatorText("Échantillonnage");
        changed |= ImGui::SliderScalar("Multisampling", ImGuiDataType_U32, &p.multisampling_size, &one, &max_ms);
        changed |= ImGui::SliderScalar("Random sampling", ImGuiDataType_U32, &p.random_sampling_size, &one, &max_rs);

        ImGui::SeparatorText("Lumière");
        changed |= ImGui::DragFloatRange2("Lambda min/max", &p.lambda_min, &p.lambda_max, 0.005f, 0.f, 1.f);
        changed |= ImGui::SliderFloat("Atténuation", &p.attenuation_coef, 0.f, 1.f, "%.4f", ImGuiSliderFlags_Logarithmic);

        ImGui::SeparatorText("Fun");
        changed |= ImGui::SliderFloat("PI", &p.pi, 0.f, 10.f);

        if (ImGui::Button("Reset")) {
            p = RaytracerParams{};
            changed = true;
        }
        if (changed) {
            context->screen.resetTextures();
        }

        ImGui::End();
    }
}

void UI::render() {
    ImGui::Render();
}

void UI::stop() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
