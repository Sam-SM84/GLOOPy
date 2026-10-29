#include "Camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 dir)
{
    position = pos;
    direction = glm::normalize(dir);
    yaw = -90.0f;
    pitch = 0.0f;
}

glm::mat4 Camera::getProjectionMatrix(float FOV, float aspect, float near, float far)
{
    return glm::perspective(glm::radians(FOV), aspect, near, far);
}

glm::mat4 Camera::getViewMatrix()
{
    return glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
}