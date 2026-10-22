#include "OpenGLIncludes.h"
#include "Parameters.h"
#include "Globals.h"
#include "Context.h"
#include "ObjectManager.h"
#include "Mesh.h"
#include "MeshHelpers.h"
#include "UI.h"

extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

std::shared_ptr<Context> context;

double currentTime = 0.0f;
double deltaTime = 0.0f;
void ProcessInput(GLFWwindow* window)
{
    context->camera.processKeyboard(window,deltaTime);
}

void start() {
    // make the cursor disappear on focusing the window
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    context = std::make_shared<Context>(Context{
        .camera = {(float) WINDOW_WIDTH, (float) WINDOW_HEIGHT, 1.f, glm::radians(70.f), 1, 100},
        .screen = {},
        .objectManager = {}
    });
    context->camera.can_move = true;
    // setting the window user pointer
    glfwSetWindowUserPointer(window, context.get());

    // setting up the mesh scene
    auto& manager = context->objectManager;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::shared_ptr<Mesh> mesh;

    MeshHelpers::generateCube(vertices, indices);
    mesh = std::make_shared<Mesh>(vertices, indices);
    mesh->transform.position = glm::vec3(0.0f, 2.0f, 0.0f);
    mesh->material.id = MaterialType::Light;
    mesh->material.intensity = 15.f;
    mesh->material.albedo.fill(1.f);
    mesh->material.T = 5000.f;
    manager.addMesh(mesh);

    std::shared_ptr<Sphere> sphere;

    MeshHelpers::generateFromFile("../assets/diamond.fbx", vertices, indices);
    mesh = std::make_shared<Mesh>(vertices, indices);
    mesh->transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    mesh->transform.rotation = glm::vec3(-glm::radians(53.0f), 0.0f, 0.0f);
    mesh->material.id = MaterialType::Glass;
    mesh->material.albedo.fill(1.f);
    mesh->material.n = N_SAPPHIRE;
    mesh->material.k = K_SAPPHIRE;
    mesh->material.metal_fuzz = 0.3f;
    manager.addMesh(mesh);

    MeshHelpers::generateFromFile("../assets/diamond.fbx", vertices, indices);
    mesh = std::make_shared<Mesh>(vertices, indices);
    mesh->transform.position = glm::vec3(3.0f, -0.35f, 0.0f);
    mesh->transform.rotation = glm::vec3(-glm::radians(53.0f), 0.0f, 0.0f);
    mesh->material.id = MaterialType::Glass;
    mesh->material.albedo.fill(1.f);
    mesh->material.n = N_DIAMOND;
    mesh->material.k = K_DIAMOND;
    mesh->material.metal_fuzz = 0.3f;
    manager.addMesh(mesh);

    sphere = std::make_shared<Sphere>(glm::vec3(0.0f, -1001.0f, -2.5f), 1000.f);
    sphere->material.id = MaterialType::Lambertian;
    sphere->material.albedo.fill(1.f);
    sphere->material.metal_fuzz = 0.3f;
    manager.addSphere(sphere);
    manager.buildBVH(); // it is weird that this function must be before buildSSBOs...
    manager.buildSSBOs();
}

void update(){
    deltaTime = glfwGetTime() - currentTime;
    currentTime = glfwGetTime();
}

int main()
{
    try {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Construct the window
        window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL Template", nullptr, nullptr);
        if (!window)
        {
            std::cout << "Failed to create the GLFW window\n";
            glfwTerminate();
        }

        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD\n";
            return -1;
        }

        // Handle view port dimensions
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
        {
            const auto context = static_cast<Context*>(glfwGetWindowUserPointer(window));
            glViewport(0, 0, width, height);
            WINDOW_WIDTH = width;
            WINDOW_HEIGHT = height;
            context->camera.aspectRatio = (float) width / (float) height;
            context->screen.windowResizeCallback(window, width, height);
        });

        // setting the cursor position callback
        glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
            const auto context = static_cast<Context*>(glfwGetWindowUserPointer(window));
            context->camera.mouseCallback(window, xpos, ypos);
        });

        // setting the keyboard callbacks
        glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
            auto* ctx = static_cast<Context*>(glfwGetWindowUserPointer(w));
            if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
                ctx->screen.showUI = !ctx->screen.showUI;
            }
            else if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
                ctx->camera.can_move = !ctx->camera.can_move;
                glfwSetInputMode(w, GLFW_CURSOR,
                    ctx->camera.can_move ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            }
            else if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
                ctx->screen.resetTextures();
            }
        });

        // setting up ImGui
        UI ui{};
        ui.start();

        // modifying variables before runtimes
        start();

        // This is the render loop
        while (!glfwWindowShouldClose(window))
        {
            // Druids are the best
            glClearColor(1.00f, 0.49f, 0.04f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);
            ProcessInput(window);

            update();
            ui.update();

            context->screen.draw();
            ui.render();

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        context.reset();
        ui.stop();
        glfwTerminate();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}