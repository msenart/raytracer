#include "UI.h"
#include "Globals.h"
#include "Context.h"
#include "MeshHelpers.h"
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

    sceneWindow(*context);

    if (context->screen.mode != RenderMode::Raytracing) return;

    ImGui::Begin("Raytracing global parameters");
    ImGui::SeparatorText("Rendering");
    changed |= ImGui::SliderScalar("Max iterations", ImGuiDataType_U32, &p.max_iter, &one, &max_iter_cap);
    changed |= ImGui::SliderFloat("Max distance", &p.max_distance, 1.f, 10000.f, "%.0f", ImGuiSliderFlags_Logarithmic);
    changed |= ImGui::SliderFloat("Background", &p.background_intensity, 0.f, 5.f);
    changed |= ImGui::SliderFloat("Epsilon", &p.eps, 1e-8f, 1e-1f, "%.7f", ImGuiSliderFlags_Logarithmic);
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

void UI::sceneWindow(Context& context) {
    auto& manager = context.objectManager;
    const bool rasterization = context.screen.mode == RenderMode::Rasterization;

    ImGui::Begin("Scene");
    ImGui::SeparatorText("Render mode (F4)");
    if (ImGui::RadioButton("Rasterization", rasterization)) context.setRenderMode(RenderMode::Rasterization);
    ImGui::SameLine();
    if (ImGui::RadioButton("Raytracing", !rasterization)) context.setRenderMode(RenderMode::Raytracing);

    if (rasterization) addRemoveControls(context);

    ImGui::SeparatorText("Selection");
    Mesh* mesh = manager.selectedMesh();
    Sphere* sphere = manager.selectedSphere();
    if (!mesh && !sphere) {
        ImGui::TextWrapped("Click an object to select it (press F2 to free the cursor).");
        ImGui::End();
        return;
    }
    ImGui::Text("%s %u", mesh ? "Mesh" : "Sphere", manager.selection.index);

    if (rasterization) {
        ImGui::SeparatorText("Transform");
        if (mesh) {
            if (eulerOwner != mesh) {
                eulerOwner = mesh;
                eulerDegrees = glm::degrees(glm::eulerAngles(mesh->transform.rotation));
            }
            ImGui::DragFloat3("Position", glm::value_ptr(mesh->transform.position), 0.05f);
            if (ImGui::DragFloat3("Rotation", glm::value_ptr(eulerDegrees), 0.5f)) {
                mesh->transform.rotation = glm::quat(glm::radians(eulerDegrees));
            }
            ImGui::DragFloat3("Scale", glm::value_ptr(mesh->transform.scale), 0.01f, 0.001f, 1000.f);
        }
        else {
            ImGui::DragFloat3("Position", glm::value_ptr(sphere->pos), 0.05f);
            ImGui::DragFloat("Radius", &sphere->radius, 0.05f, 0.001f, 100000.f);
        }
    }
    else {
        ImGui::SeparatorText("Material");
        materialEditor(context, mesh ? mesh->material : sphere->material);
    }
    ImGui::End();
}

void UI::addRemoveControls(Context& context) {
    auto& manager = context.objectManager;
    ImGui::SeparatorText("Add / remove");

    // new objects appear in front of the camera
    const glm::vec3 spawn = context.camera.position + glm::normalize(context.camera.center - context.camera.position) * 5.f;
    const auto addMesh = [&](std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) {
        auto mesh = std::make_shared<Mesh>(vertices, indices);
        mesh->transform.position = spawn;
        manager.addMesh(mesh);
        manager.selection = {Selection::Kind::Mesh, manager.meshesNumber() - 1};
    };

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    if (ImGui::Button("Cube")) {
        MeshHelpers::generateCube(vertices, indices);
        addMesh(vertices, indices);
    }
    ImGui::SameLine();
    if (ImGui::Button("Plane")) {
        MeshHelpers::generatePlane(vertices, indices, glm::vec2(1.f), true);
        addMesh(vertices, indices);
    }
    ImGui::SameLine();
    if (ImGui::Button("Mesh sphere")) {
        MeshHelpers::generateSphere(vertices, indices);
        addMesh(vertices, indices);
    }
    ImGui::SameLine();
    if (ImGui::Button("Sphere")) {
        manager.addSphere(std::make_shared<Sphere>(spawn, 1.f));
        manager.selection = {Selection::Kind::Sphere, manager.spheresNumber() - 1};
    }

    ImGui::InputText("Model file", modelPath, sizeof(modelPath));
    if (ImGui::Button("Load model")) {
        try {
            MeshHelpers::generateFromFile(modelPath, vertices, indices);
            addMesh(vertices, indices);
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    ImGui::SameLine();
    const bool hasSelection = manager.selectedMesh() || manager.selectedSphere();
    ImGui::BeginDisabled(!hasSelection);
    if (ImGui::Button("Remove selected")) {
        if (manager.selectedMesh()) manager.removeMesh(manager.selection.index);
        else manager.removeSphere(manager.selection.index);
    }
    ImGui::EndDisabled();
}

void UI::materialEditor(Context& context, Material& material) {
    struct Preset { const char* name; const std::array<float, N_BINS>* n; const std::array<float, N_BINS>* k; };
    static const Preset presets[] = {
        {"Diamond",     &N_DIAMOND,     &K_DIAMOND},
        {"Sapphire",    &N_SAPPHIRE,    &K_SAPPHIRE},
        {"Ruby",        &N_RUBY,        &K_RUBY},
        {"Gold",        &N_GOLD,        &K_GOLD},
        {"Aquamarine",  &N_AQUAMARINE,  &K_AQUAMARINE},
        {"Copper",      &N_COPPER,      &K_COPPER},
        {"Rutile",      &N_RUTILE,      &K_RUTILE},
        {"Alexandrite", &N_ALEXANDRITE, &K_ALEXANDRITE},
    };
    static const char* types[] = {"Light", "Lambertian", "Metallic", "Glass"};
    const auto mean = [](const std::array<float, 16>& values) {
        return std::accumulate(values.begin(), values.end(), 0.f) / static_cast<float>(values.size());
    };

    bool changed = false;

    int type = static_cast<int>(material.id);
    if (ImGui::Combo("Type", &type, types, IM_ARRAYSIZE(types))) {
        material.id = static_cast<MaterialType>(type);
        changed = true;
    }

    float albedo = mean(material.albedo);
    if (ImGui::SliderFloat("Albedo", &albedo, 0.f, 1.f)) {
        material.albedo.fill(albedo);
        changed = true;
    }

    if (material.id == MaterialType::Light) {
        changed |= ImGui::SliderFloat("Intensity", &material.intensity, 0.f, 100.f);
        changed |= ImGui::SliderFloat("Temperature (K)", &material.T, 1000.f, 12000.f);
    }
    else {
        changed |= ImGui::SliderFloat("Fuzz", &material.metal_fuzz, 0.f, 1.f);
    }

    if (material.id == MaterialType::Metallic || material.id == MaterialType::Glass) {
        if (ImGui::BeginCombo("Preset (n, k)", "Select...")) {
            for (const auto& preset : presets) {
                if (ImGui::Selectable(preset.name)) {
                    material.n = *preset.n;
                    material.k = *preset.k;
                    changed = true;
                }
            }
            ImGui::EndCombo();
        }
        // moving the mean while keeping the dispersion
        float n = mean(material.n);
        if (ImGui::SliderFloat("Mean n", &n, 0.1f, 4.f)) {
            const float shift = n - mean(material.n);
            for (auto& v : material.n) v = std::max(v + shift, 0.01f);
            changed = true;
        }
        float k = mean(material.k);
        if (ImGui::SliderFloat("Mean k", &k, 0.f, 5.f)) {
            const float shift = k - mean(material.k);
            for (auto& v : material.k) v = std::max(v + shift, 0.f);
            changed = true;
        }
    }

    if (changed) {
        context.objectManager.updateMaterials();
        context.screen.resetTextures();
    }
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
