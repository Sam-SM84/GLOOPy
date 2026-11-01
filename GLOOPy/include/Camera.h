#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
class Camera
{
public:
    glm::vec3 position;
    glm::vec3 direction;
    float yaw;
    float pitch;
    float speed;

    Camera(glm::vec3 pos, glm::vec3 dir);

    glm::mat4 getProjectionMatrix(float FOV, float aspect, float near, float far);

    glm::mat4 getViewMatrix();

    void updateDirection();
    void updateAngles(float x, float y, float sensitivity);
};