#pragma once

#include <map>

#include "BaseObject.hpp"

typedef int KeyboardListenerPriority; // [-10000, 10000]
static const int KEY_LOWEST_PRIORITY = -10000;
static const int KEY_NORMAL_PRIORITY = 0; // When object won't cancel keys and don't care the call order
static const int KEY_HIGHEST_PRIORITY = 10000;

class KeyboardListenerObject : public BaseObject
{
public:
    virtual const char *GetObjectTypeName()
    {
        return "KeyboardListenerObject";
    }
    virtual std::map<int, KeyboardListenerPriority> KeyboardRegister() = 0;
    virtual bool KeyboardCallback(int key, int state) = 0;
};
