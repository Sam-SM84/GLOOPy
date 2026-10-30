#include "FrameTime.h"

Time::Time()
{
    t1 = (float)glfwGetTime();
    t2 = 0;
    deltaTime = 0;
}

void Time:: update()
{
    t2 = (float)glfwGetTime();
    deltaTime = t2 - t1;
    t1 = t2;
}