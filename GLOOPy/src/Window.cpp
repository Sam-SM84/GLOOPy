#include "Window.h"
#include <iostream>

void Window::initialize(int majorVersion, int minorVersion)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, majorVersion);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minorVersion);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

Window::Window(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)
{
    screen = glfwCreateWindow(width, height, title, monitor, share);
    if (screen == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(screen);
}


void Window::screenSizeUpdaterFunction(GLFWframebuffersizefun function)
{
    glfwSetFramebufferSizeCallback(screen, function);
}

void Window::screenMouseMotionFunction(GLFWcursorposfun function)
{
    glfwSetCursorPosCallback(screen, function);
}

void Window::screenMouseScrollFunction(GLFWscrollfun function)
{
    glfwSetScrollCallback(screen, function);
}

void Window::beginFrame(float r, float g, float b, float a, GLbitfield mask)
{
    glClearColor(r, g, b, a);
    glClear(mask);
}

void Window::endFrame()
{
    glfwSwapBuffers(screen);
    glfwPollEvents();
}

void Window::inputMode(int mode, int value)
{
    glfwSetInputMode(screen, mode, value);
}