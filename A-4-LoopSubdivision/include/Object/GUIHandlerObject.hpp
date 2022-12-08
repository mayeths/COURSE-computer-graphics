#pragma once

#include <GLFW/glfw3.h>

#include "BaseObject.hpp"

class GUIHandlerObject : public BaseObject
{
public:
    virtual void refresh(double now, double lastTime) {}
    virtual const char *GetObjectTypeName() { return "GUIHandlerObject"; }
};

