#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform
{
public:
    glm::mat4 modelMatrix;

    Transform();

    void moveObject(glm::vec3 position);

    void rotateObject(float angle, glm::vec3 axis);

    void scaleObject(glm::vec3 scale);
};