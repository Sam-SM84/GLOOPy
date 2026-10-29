#include "Transform.h"

Transform::Transform()
{
    modelMatrix = glm::mat4(1.0f);
}

void Transform::moveObject(glm::vec3 position)
{
    modelMatrix = glm::translate(modelMatrix, position);
}

void Transform::rotateObject(float angle, glm::vec3 axis)
{
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), axis);
}

void Transform::scaleObject(glm::vec3 scale)
{
    modelMatrix = glm::scale(modelMatrix, scale);
}