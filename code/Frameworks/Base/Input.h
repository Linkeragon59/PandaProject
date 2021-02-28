#pragma once

#include <iostream>

enum MouseInput
{
    Button1 = 0,
    Button2 = 1
};

class InputManager
{
public:
    static void Create()
    {
        ourInstance = new InputManager;
    }
    static void Destroy()
    {
        delete ourInstance;
        ourInstance = nullptr;
    }
    static InputManager* GetInstance() { return ourInstance; }

    bool PollInput(MouseInput);

private:
    static InputManager* ourInstance;
};
