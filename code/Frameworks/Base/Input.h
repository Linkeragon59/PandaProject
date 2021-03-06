#pragma once

#include <vector>
struct GLFWwindow;

namespace Input
{
    enum class RawInput
    {
        MouseLeft,
        MouseRight,

		KeyA,
		KeyD,
		KeyI,
		KeyS,
		KeyW,

		KeySpace,
		KeyEscape,
    };

    enum class RawInputState
    {
        Unknown,

        Pressed,
        Released,
        Repeated,
    };

    class InputManager
    {
    public:
        InputManager();
        ~InputManager();

        static void Create();
        static void Destroy();
        static InputManager* GetInstance() { return ourInstance; }

        void AddWindow(GLFWwindow* aWindow) { myWindows.push_back(aWindow); }
        void RemoveWindow(GLFWwindow* aWindow) { std::erase(myWindows, aWindow); }

        RawInputState PollRawInput(RawInput anInput, unsigned int aWindowIdx = 0);
        void PollMousePosition(double& anOutX, double& anOutY, unsigned int aWindowIdx = 0);

    private:
        static InputManager* ourInstance;

        std::vector<GLFWwindow*> myWindows;
    };
} // namespace Base
