#pragma once

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

        int PollMouseInput(GLFWwindow* window, MouseInput anInput);
        double PollMousePosition(GLFWwindow* window);

    private:
        static InputManager* ourInstance;
    };
} // namespace Base
