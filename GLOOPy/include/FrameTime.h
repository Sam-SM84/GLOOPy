#pragma once

#include <glad/glad.h>
#include<GLFW/glfw3.h>

class Time
{
    float t1;
    float t2;

public:
    float deltaTime;

    Time();

    void update();
};