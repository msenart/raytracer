#include "Camera.h"
#include "OpenGLIncludes.h"
#include "Parameters.h"
#include "Bufwrap.h"
#include "Screen.h"
#include "Shader.h"
#include "Globals.h"

Camera camera{glm::radians(90.f),(float) WINDOW_WIDTH,(float) WINDOW_HEIGHT,1,100};
double currentTime = 0.0f;
double deltaTime = 0.0f;
std::shared_ptr<Screen> screen;
void ProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    camera.processKeyboard(window,deltaTime);
}

void start() {
    camera.can_move = true;

    // make the cursor disappear on focusing the window
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    screen = std::make_shared<Screen>();
}

void update(){
    deltaTime = glfwGetTime() - currentTime;
    currentTime = glfwGetTime();

    screen->draw();
}

int main()
{
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

    // setting the window user pointer
    glfwSetWindowUserPointer(window, &camera);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    // Handle view port dimensions
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
        WINDOW_WIDTH = width;
        WINDOW_HEIGHT = height;
        camera.aspectRatio = (float) width / (float) height;
    });

    // setting the cursor position callback
    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double xpos, double ypos) {
        static_cast<Camera*>(glfwGetWindowUserPointer(w))->mouseCallback(w, xpos, ypos);
    });

    ShaderProgram::allEnableVariableNotBoundMessage(false);
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

    glfwTerminate();
    return 0;
}