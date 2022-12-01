#pragma once

#include <map>
#include <limits>

#include "BaseObject.hpp"

static const int KEY_LOWEST_PRIORITY = std::numeric_limits<int>::min();
static const int KEY_NORMAL_PRIORITY = 0; // When object won't cancel keys and don't care the call order
static const int KEY_HIGHEST_PRIORITY = std::numeric_limits<int>::max();

enum class KeyState : bool
{
    STATE_RELEASE, STATE_PRESS
};
enum class KeyEvent : char {
    KEY_DOWN, KEY_UP, KEY_PRESS
};
typedef int KeyCode;

class KeyPriority {
public:
    int priority = 0;
    KeyPriority(int priority = 0) {
        this->priority = priority;
    }
    static KeyPriority Highest() {
        return KeyPriority( std::numeric_limits<int>::max());
    }
    static KeyPriority Normal() {
        return KeyPriority(0);
    }
    static KeyPriority Lowest() {
        return KeyPriority( std::numeric_limits<int>::min());
    }
};

class KeyboardListenerObject : public BaseObject
{
public:
    virtual const char *GetObjectTypeName()
    {
        return "KeyboardListenerObject";
    }
    virtual std::map<int, KeyPriority> KeyboardRegister() = 0;
    virtual bool KeyboardCallback(int key, KeyEvent event) = 0;
};
