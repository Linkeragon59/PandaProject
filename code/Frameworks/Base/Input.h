#pragma once

#include <vector>
#include <functional>
struct GLFWwindow;

namespace Input
{	
	enum class RawInput
	{
		MouseStart,
		MouseLeft = MouseStart,
		MouseRight,
		MouseMiddle,
		MouseEnd = MouseMiddle,

		KeyNumStart,
		Key0 = KeyNumStart,
		Key1,
		Key2,
		Key3,
		Key4,
		Key5,
		Key6,
		Key7,
		Key8,
		Key9,
		KeyNumEnd = Key9,

		KeyAlphaStart,
		KeyA = KeyAlphaStart,
		KeyB,
		KeyC,
		KeyD,
		KeyE,
		KeyF,
		KeyG,
		KeyH,
		KeyI,
		KeyJ,
		KeyK,
		KeyL,
		KeyM,
		KeyN,
		KeyO,
		KeyP,
		KeyQ,
		KeyR,
		KeyS,
		KeyT,
		KeyU,
		KeyV,
		KeyW,
		KeyX,
		KeyY,
		KeyZ,
		KeyAlphaEnd = KeyZ,

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
		GLFWwindow* myWindow;
		std::function<void()> myCallback;
	};

	struct ScrollInputCallback
	{
		GLFWwindow* myWindow;
		std::function<void(double, double)> myCallback;
	};

	class InputManager
	{
	public:
		static void Create();
		static void Destroy();
		static InputManager* GetInstance() { return ourInstance; }

		void AddWindow(GLFWwindow* aWindow);
		void RemoveWindow(GLFWwindow* aWindow);

		RawInputState PollRawInput(RawInput anInput, uint aWindowIdx = 0) const;
		void PollMousePosition(double& anOutX, double& anOutY, uint aWindowIdx = 0) const;

		uint AddCallback(RawInput anInput, std::function<void()> aCallback, uint aWindowIdx = 0);
		void RemoveCallback(uint aCallbakId);

		uint AddScrollCallback(std::function<void(double, double)> aCallback, uint aWindowIdx = 0);
		void RemoveScrollCallback(uint aCallbakId);

		static void KeyCallback(GLFWwindow* aWindow, int aKey, int aScanCode, int anAction, int someMods);
		static void MouseCallback(GLFWwindow* aWindow, int aButton, int anAction, int someMods);
		static void ScrollCallback(GLFWwindow* aWindow, double anX, double anY);

	private:
		static InputManager* ourInstance;

		std::vector<GLFWwindow*> myWindows;
		std::vector<InputCallback> myInputCallbacks;
		std::vector<ScrollInputCallback> myScrollInputCallbacks;
	};
} // namespace Input
