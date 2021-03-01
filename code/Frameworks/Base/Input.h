#pragma once

#include <iostream>

struct GLFWwindow;

namespace Base
{
    enum MouseInput
    {
        Button1 = 0,
        Button2 = 1
    };

    class InputManager
    {
    public:
        InputManager();
        ~InputManager();

        static void Create();
        static void Destroy();
        static InputManager* GetInstance() { return ourInstance; }

        void PollInput(GLFWwindow* window, MouseInput anInput);
        
    private:
        static InputManager* ourInstance;
    };
} // namespace Base
