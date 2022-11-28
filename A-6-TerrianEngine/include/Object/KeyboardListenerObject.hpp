#pragma once

#include <map>
#include <limits>

#include "BaseObject.hpp"

typedef int KeyboardListenerPriority; // [-10000, 10000]
static const int KEY_LOWEST_PRIORITY = std::numeric_limits<int>::min();
static const int KEY_NORMAL_PRIORITY = 0; // When object won't cancel keys and don't care the call order
static const int KEY_HIGHEST_PRIORITY = std::numeric_limits<int>::max();

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
