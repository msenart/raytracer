#include "OpenGLIncludes.h"
#include "Parameters.h"
#include "Globals.h"
#include "Context.h"

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
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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