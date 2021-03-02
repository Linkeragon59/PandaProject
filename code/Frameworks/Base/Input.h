#pragma once

struct GLFWwindow;

// We should eventually use the GLM class for this, but I'm
// not sure yet how to include it safely
struct vec2
{
    double x;
    double y;
};

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
        vec2 PollMousePosition(GLFWwindow* window);

    private:
        static InputManager* ourInstance;
    };
} // namespace Base
