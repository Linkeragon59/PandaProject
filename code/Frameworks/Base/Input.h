#pragma once

#include <vector>
#include <functional>
struct GLFWwindow;

namespace Input
{	
    enum class RawInput
    {
        MouseLeft,
        MouseRight,
        MouseMiddle,

        MouseEnd = MouseMiddle,

		KeyA,
		KeyB,
		KeyC,
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

	struct InputCallback
	{
		RawInput myInput;
		std::function<void()> myCallback;
	};

    struct ScrollInputCallback
    {
        std::function<void(double, double)> myCallback;
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

        void AddCallback(RawInput anInput, std::function<void()> aCallback);
        void RemoveCallback(RawInput anInput, std::function<void()> aCallback);

        void AddScrollInputCallback(std::function<void(double, double)> aCallback);

		void SetupCallback(unsigned int aWindowIdx = 0);

    private:
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MouseCallback(GLFWwindow* window, int button, int action, int mods);
        static void ScrollCallback(GLFWwindow* window, double x, double y);

        static InputManager* ourInstance;
        std::vector<GLFWwindow*> myWindows;
        std::vector<InputCallback> myInputCallbacks;
        std::vector<ScrollInputCallback> myScrollInputCallbacks;
    };
} // namespace Input
