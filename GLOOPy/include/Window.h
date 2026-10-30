#pragma once

#include <glad/glad.h>
#include<GLFW/glfw3.h>

class Window
{
public:
    GLFWwindow* screen;

    static void initialize(int majorVersion, int minorVersion);

    Window(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share);

    void screenSizeUpdaterFunction(GLFWframebuffersizefun function);
    void screenMouseMotionFunction(GLFWcursorposfun function);
    void screenMouseScrollFunction(GLFWscrollfun function);
    void beginFrame(float r, float g, float b, float a, GLbitfield mask);
    void endFrame();
    void inputMode(int mode,int value);

};