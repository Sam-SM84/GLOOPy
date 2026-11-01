#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"

class Input
{
	static bool isKeyboardPressed(GLFWwindow* window, int value);
	static bool isKeyboardReleased(GLFWwindow* window, int value);
	static bool isMouseButtonPressed(GLFWwindow* window, int value);
	static bool isMouseButtonReleased(GLFWwindow* window, int value);
};
