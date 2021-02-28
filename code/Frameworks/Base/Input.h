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

        void InitWindow();
        void Cleanup();

        void PollInput(MouseInput anInput);
        void MouseButtonCallback(int button, int action);

    private:
        static InputManager* ourInstance;
        GLFWwindow* myWindow = nullptr;
    };
} // namespace Base
