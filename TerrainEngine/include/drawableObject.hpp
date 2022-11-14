#pragma once
#include "sceneElements.hpp"

class drawableObject
{
public:
    virtual void draw() = 0;
    virtual void setGui() {};
    virtual void update() {};

    static sceneElements* scene;
};
