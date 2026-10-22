#include "UI.h"
#include "Globals.h"
#include "Context.h"
void UI::start() {
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // store hardware accelerator specs
    vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    setupImGuiStyle();
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

    if (!context->screen.showUI) return;

    bool changed = false;
    const unsigned int one = 1, max_iter_cap = 500, max_ms = 16, max_rs = 16;

    ImGui::Begin("Rendering performances");
    ImGui::SeparatorText("Hardware");
    ImGui::Text("Renderer: %s", renderer.c_str());
    ImGui::Text("Version: %s",  version.c_str());
    ImGui::Text("Vendor: %s",   vendor.c_str());
    ImGui::SeparatorText("Performances");
    ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
    auto primitives_number = context->objectManager.primitivesNumber();
    ImGui::Text("Primitives: %u", primitives_number);
    ImGui::End();

    ImGui::Begin("Raytracing global parameters");
    ImGui::SeparatorText("Rendering");
    changed |= ImGui::SliderScalar("Max iterations", ImGuiDataType_U32, &p.max_iter, &one, &max_iter_cap);
    changed |= ImGui::SliderFloat("Max distance", &p.max_distance, 1.f, 10000.f, "%.0f", ImGuiSliderFlags_Logarithmic);
    changed |= ImGui::SliderFloat("Background", &p.background_intensity, 0.f, 5.f);
    changed |= ImGui::SliderFloat("Epsilon", &p.eps, 1e-7f, 1e-1f, "%.7f", ImGuiSliderFlags_Logarithmic);
    ImGui::SeparatorText("Sampling");
    changed |= ImGui::SliderScalar("Random sampling ray/frame", ImGuiDataType_U32, &p.random_sampling_size, &one, &max_rs);
    ImGui::SeparatorText("Light");
    changed |= ImGui::DragFloatRange2("Lambda min/max", &p.lambda_min, &p.lambda_max, 0.005f, 0.f, 1.f);
    changed |= ImGui::SliderFloat("Attenuation", &p.attenuation_coef, 0.f, 1.f, "%.4f", ImGuiSliderFlags_Logarithmic);
    ImGui::SeparatorText("Fun");
    changed |= ImGui::SliderFloat("Pi", &p.pi, 0.f, 10.f);
    if (ImGui::Button("Reset")) {
        p = RaytracerParams{};
        changed = true;
    }
    if (changed) {
        context->screen.resetTextures();
    }
    ImGui::End();
}

void UI::setupImGuiStyle()
{
    bool bStyleDark_ = true;
    float alpha_ = 1.f;

    ImGuiStyle& style = ImGui::GetStyle();

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("../fonts/Roboto-Medium.ttf", 15.f);

    // light style from Pacôme Danhiez (user itamago) https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
    style.Alpha = 1.0f;
    style.FrameRounding = 3.0f;
    style.Colors[ImGuiCol_Text]                  = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
    style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_PopupBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
    style.Colors[ImGuiCol_Border]                = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
    style.Colors[ImGuiCol_FrameBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Button]                = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Header]                = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);

    if( bStyleDark_ )
    {
        for (int i = 0; i <= ImGuiCol_COUNT; i++)
        {
            ImVec4& col = style.Colors[i];
            float H, S, V;
            ImGui::ColorConvertRGBtoHSV( col.x, col.y, col.z, H, S, V );

            if( S < 0.1f )
            {
               V = 1.0f - V;
            }
            ImGui::ColorConvertHSVtoRGB( H, S, V, col.x, col.y, col.z );
            if( col.w < 1.00f )
            {
                col.w *= alpha_;
            }
        }
    }
    else
    {
        for (int i = 0; i <= ImGuiCol_COUNT; i++)
        {
            ImVec4& col = style.Colors[i];
            if( col.w < 1.00f )
            {
                col.x *= alpha_;
                col.y *= alpha_;
                col.z *= alpha_;
                col.w *= alpha_;
            }
        }
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
