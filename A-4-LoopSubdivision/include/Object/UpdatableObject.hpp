#pragma once

#include <GLFW/glfw3.h>

#include "BaseObject.hpp"

class UpdatableObject : public BaseObject
{
public:
    virtual void update(double now, double lastUpdateTime, GLFWwindow *window) = 0;
    virtual const char *GetObjectTypeName() { return "UpdatableObject"; }
};

