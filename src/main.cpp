#include "OpenGLIncludes.h"
#include "Parameters.h"
#include "Globals.h"
#include "Context.h"
#include "MeshManager.h"
#include "Mesh.h"
#include "MeshHelpers.h"

extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

std::shared_ptr<Context> context;

double currentTime = 0.0f;
double deltaTime = 0.0f;
void ProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    context->camera.processKeyboard(window,deltaTime);
}

void start() {
    // make the cursor disappear on focusing the window
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    Context ctx = {
        .camera = {glm::radians(90.f),(float) WINDOW_WIDTH,(float) WINDOW_HEIGHT,1,100},
        .screen = {}
    };
    context = std::make_shared<Context>(ctx);
    context->camera.can_move = true;
    // setting the window user pointer
    glfwSetWindowUserPointer(window, context.get());

    // display hardware accelerator specs
    std::cout << "Vendor: "   << glGetString(GL_VENDOR)   << "\n";
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "Version: "  << glGetString(GL_VERSION)  << "\n";

    // setting up the mesh scene

    const std::array<float,16> DIAMOND_R0 = {
        0.1786f, 0.1775f, 0.1764f, 0.1756f,
        0.1748f, 0.1741f, 0.1736f, 0.1731f,
        0.1726f, 0.1722f, 0.1718f, 0.1715f,
        0.1712f, 0.1709f, 0.1707f, 0.1704f
    };



    MeshManager manager;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::shared_ptr<Mesh> mesh;

    MeshHelpers::generateCube(vertices, indices);
    mesh = std::make_shared<Mesh>(vertices, indices);
    mesh->transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    mesh->material.id = MaterialType::Light;
    mesh->material.reflectance.fill(1.f);
    mesh->material.B = glm::vec3(4.8669e-4, 5.8546e-5, 0.0);
    mesh->material.C = glm::vec3(0.0042014, 0.017433, 1.0);
    manager.addMesh(mesh);
    //
    // MeshHelpers::generateSphere(vertices, indices);
    // mesh = std::make_shared<Mesh>(vertices, indices);
    // mesh->transform.position = glm::vec3(2.5f, 0.0f, 0.0f);
    // mesh->material.id = MaterialType::Glass;
    // mesh->material.color = glm::vec4(1.f);
    // mesh->material.eta = 1.5f;
    // manager.addMesh(mesh);
    //
    // MeshHelpers::generateSphere(vertices, indices);
    // mesh = std::make_shared<Mesh>(vertices, indices);
    // mesh->transform.position = glm::vec3(-2.5f, 0.0f, 0.0f);
    // mesh->material.id = MaterialType::Glass;
    // mesh->material.color = glm::vec4(1.f);
    // mesh->material.eta = 1.5f;
    // mesh->material.metal_fuzz = 0.3f;
    // manager.addMesh(mesh);

    std::shared_ptr<Sphere> sphere;

    // sphere = std::make_shared<Sphere>(glm::vec3(0.0f, 0.01f, 2.5f), 1.f);
    // sphere->material.id = MaterialType::Metallic;
    // sphere->material.color = glm::vec4(1.f, 0.f, 0.f, 1.f);
    // sphere->material.eta = 1.5f;
    // sphere->material.metal_fuzz = 0.3f;
    // manager.addSphere(sphere);
    //
    sphere = std::make_shared<Sphere>(glm::vec3(0.0f, 0.01f, -2.5f), 1.f);
    sphere->material.id = MaterialType::Glass;
    sphere->material.reflectance = DIAMOND_R0;
    sphere->material.B = glm::vec3(4.3356, 0.3306, 0.0);
    sphere->material.C = glm::vec3(0.011236, 0.030625, 1.0);
    sphere->material.metal_fuzz = 0.3f;
    manager.addSphere(sphere);

    sphere = std::make_shared<Sphere>(glm::vec3(0.0f, -1001.0f, -2.5f), 1000.f);
    sphere->material.id = MaterialType::Lambertian;
    sphere->material.reflectance.fill(1.f);
    sphere->material.B = glm::vec3(4.8669e-4, 5.8546e-5, 0.0);
    sphere->material.C = glm::vec3(0.0042014, 0.017433, 1.0);
    sphere->material.metal_fuzz = 0.3f;
    manager.addSphere(sphere);

    manager.buildSSBOs();
}

void update(){
    deltaTime = glfwGetTime() - currentTime;
    currentTime = glfwGetTime();

    context->screen.draw();
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

        // modifying variables before runtimes
        start();

        // This is the render loop
        while (!glfwWindowShouldClose(window))
        {
            ProcessInput(window);
            // Druids are the best
            glClearColor(1.00f, 0.49f, 0.04f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);

            update();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        context.reset();
        glfwTerminate();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}