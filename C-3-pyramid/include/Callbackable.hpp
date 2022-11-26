#pragma once

class Callbackable
{
public:

    virtual bool SubscribeInput() { return false; }
    virtual bool SubscribeGUI() { return false; }

    virtual bool CallbackInput(int key, int state) {}
    virtual bool CallbackGUI(double lastRenderTime, double now) {}
};
