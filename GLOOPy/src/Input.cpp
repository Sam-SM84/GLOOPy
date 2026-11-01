#include "Input.h"

bool isKeyboardPressed(GLFWwindow* window,int value)
{
	return glfwGetKey(window, value) == GLFW_PRESS;
}

bool isKeyboardReleased(GLFWwindow* window, int value)
{
	return glfwGetKey(window, value) == GLFW_RELEASE;
}

bool isMouseButtonPressed(GLFWwindow* window, int value)
{
	return glfwGetMouseButton(window, value) == GLFW_PRESS;
}

bool isMouseButtonReleased(GLFWwindow* window, int value)
{
	return glfwGetMouseButton(window, value) == GLFW_RELEASE;
}
