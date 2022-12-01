#pragma once

#include "BaseObject.hpp"

class CallableObject : public BaseObject
{
public:
    virtual const char *GetObjectTypeName()
    {
        return "CallableObject";
    }
};
