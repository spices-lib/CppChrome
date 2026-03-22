#include "Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void Window::Init()
{
    GLFWwindow* window;

    if (!glfwInit()) {
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1920, 1080, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return;
    }

    m_Windows = window;
}

void Window::EventLoop(WebClient& webClient, CppRenderer& cppRenderer)
{
	while (!glfwWindowShouldClose((GLFWwindow*)m_Windows))
	{
        glfwPollEvents();
	    
        std::this_thread::sleep_for(std::chrono::seconds(1));

        cppRenderer.Render(webClient.Render());

        glfwSwapBuffers((GLFWwindow*)m_Windows);
	}

    glfwTerminate();

    glfwDestroyWindow((GLFWwindow*)m_Windows);
}