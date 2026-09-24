#include "Camera.h"
#include "OpenGLIncludes.h"
#include "Parameters.h"
#include "Bufwrap.h"
#include "Screen.h"
#include "Shader.h"
#include "Globals.h"

Camera camera{90,(float) WINDOW_WIDTH,(float) WINDOW_HEIGHT,1,100};
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
    window = glfwCreateWindow(800, 600, "OpenGL Template", nullptr, nullptr);
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
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
    });

    // setting the cursor position callback
    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double xpos, double ypos) {
        static_cast<Camera*>(glfwGetWindowUserPointer(w))->mouseCallback(w, xpos, ypos);
        std::cout << camera.position.x + camera.position.y + camera.position.z << std::endl;
        std::cout << camera.center.x + camera.center.y + camera.center.z << std::endl;
    });

    // modifying variables before runtime
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