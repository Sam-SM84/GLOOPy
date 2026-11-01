#include "Camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 dir)
{
    position = pos;
    direction = glm::normalize(dir);
    yaw = -90.0f;
    pitch = 0.0f;
    speed = 1.0f;
}

glm::mat4 Camera::getProjectionMatrix(float FOV, float aspect, float near, float far)
{
    return glm::perspective(glm::radians(FOV), aspect, near, far);
}

glm::mat4 Camera::getViewMatrix()
{
    return glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::updateDirection()
{
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction = glm::normalize(direction);
}

void Camera::updateAngles(float x, float y, float sensitivity)
{
    yaw += x * sensitivity;
    pitch += y * sensitivity;
}