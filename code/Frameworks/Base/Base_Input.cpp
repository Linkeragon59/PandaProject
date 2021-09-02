#include "Base_Input.h"

#include <GLFW/glfw3.h>

namespace Input
{
	namespace
	{
		int locRawInputToGlfwInput(RawInput aRawInput)
		{
			#define RAW2GLFW(raw, glfw) case RawInput::raw: return glfw;
			
			switch (aRawInput)
			{
			RAW2GLFW(MouseLeft, GLFW_MOUSE_BUTTON_LEFT)
			RAW2GLFW(MouseRight, GLFW_MOUSE_BUTTON_RIGHT)
			RAW2GLFW(MouseMiddle, GLFW_MOUSE_BUTTON_MIDDLE)

			RAW2GLFW(Key0, GLFW_KEY_0)
			RAW2GLFW(Key1, GLFW_KEY_1)
			RAW2GLFW(Key2, GLFW_KEY_2)
			RAW2GLFW(Key3, GLFW_KEY_3)
			RAW2GLFW(Key4, GLFW_KEY_4)
			RAW2GLFW(Key5, GLFW_KEY_5)
			RAW2GLFW(Key6, GLFW_KEY_6)
			RAW2GLFW(Key7, GLFW_KEY_7)
			RAW2GLFW(Key8, GLFW_KEY_8)
			RAW2GLFW(Key9, GLFW_KEY_9)

			RAW2GLFW(KeyA, GLFW_KEY_A)
			RAW2GLFW(KeyB, GLFW_KEY_B)
			RAW2GLFW(KeyC, GLFW_KEY_C)
			RAW2GLFW(KeyD, GLFW_KEY_D)
			RAW2GLFW(KeyE, GLFW_KEY_E)
			RAW2GLFW(KeyF, GLFW_KEY_F)
			RAW2GLFW(KeyG, GLFW_KEY_G)
			RAW2GLFW(KeyH, GLFW_KEY_H)
			RAW2GLFW(KeyI, GLFW_KEY_I)
			RAW2GLFW(KeyJ, GLFW_KEY_J)
			RAW2GLFW(KeyK, GLFW_KEY_K)
			RAW2GLFW(KeyL, GLFW_KEY_L)
			RAW2GLFW(KeyM, GLFW_KEY_M)
			RAW2GLFW(KeyN, GLFW_KEY_N)
			RAW2GLFW(KeyO, GLFW_KEY_O)
			RAW2GLFW(KeyP, GLFW_KEY_P)
			RAW2GLFW(KeyQ, GLFW_KEY_Q)
			RAW2GLFW(KeyR, GLFW_KEY_R)
			RAW2GLFW(KeyS, GLFW_KEY_S)
			RAW2GLFW(KeyT, GLFW_KEY_T)
			RAW2GLFW(KeyU, GLFW_KEY_U)
			RAW2GLFW(KeyV, GLFW_KEY_V)
			RAW2GLFW(KeyW, GLFW_KEY_W)
			RAW2GLFW(KeyX, GLFW_KEY_X)
			RAW2GLFW(KeyY, GLFW_KEY_Y)
			RAW2GLFW(KeyZ, GLFW_KEY_Z)

			RAW2GLFW(KeySpace, GLFW_KEY_SPACE)
			RAW2GLFW(KeyEscape, GLFW_KEY_ESCAPE)

			default: return GLFW_KEY_UNKNOWN;
			}

			#undef RAW2GLFW
		}

		RawInputState locGlfwInputStateToRawInputState(int aGlfwInputState)
		{
			if (aGlfwInputState == GLFW_PRESS)
				return RawInputState::Pressed;
			else if (aGlfwInputState == GLFW_RELEASE)
				return RawInputState::Released;
			else if (aGlfwInputState == GLFW_REPEAT)
				return RawInputState::Repeated;
			else
				return RawInputState::Unknown;
		}
	}

	InputManager* InputManager::ourInstance = nullptr;

	void InputManager::Create()
	{
		ourInstance = new InputManager;
	}
	
	void InputManager::Destroy()
	{
		SafeDelete(ourInstance);
	}

	RawInputState InputManager::PollRawInput(RawInput anInput, GLFWwindow* aWindow /*= nullptr*/) const
	{
		GLFWwindow* window = aWindow ? aWindow : myMainWindow;
		if (!window)
			return RawInputState::Unknown;

		if (anInput <= RawInput::MouseEnd)
			return locGlfwInputStateToRawInputState(glfwGetMouseButton(window, locRawInputToGlfwInput(anInput)));
		else
			return locGlfwInputStateToRawInputState(glfwGetKey(window, locRawInputToGlfwInput(anInput)));
	}

	void InputManager::PollMousePosition(double& anOutX, double& anOutY, GLFWwindow* aWindow /*= nullptr*/) const
	{
		GLFWwindow* window = aWindow ? aWindow : myMainWindow;
		if (!window)
		{
			anOutX = -1;
			anOutY = -1;
			return;
		}

		glfwGetCursorPos(window, &anOutX, &anOutY);
	}

	uint InputManager::AddCallback(RawInput anInput, std::function<void()> aCallback, GLFWwindow* aWindow /*= nullptr*/)
	{
		GLFWwindow* window = aWindow ? aWindow : myMainWindow;
		if (!window)
			return UINT_MAX;

		for (uint i = 0; i < myInputCallbacks.size(); ++i)
		{
			InputCallback& callback = myInputCallbacks[i];
			if (callback.myCallback == nullptr)
			{
				callback.myInput = anInput;
				callback.myWindow = window;
				callback.myCallback = aCallback;
				return i;
			}
		}

		myInputCallbacks.push_back({ anInput, window, aCallback });
		return (uint)myInputCallbacks.size() - 1;
	}

	void InputManager::RemoveCallback(uint aCallbakId)
	{
		if (aCallbakId >= myInputCallbacks.size())
			return;

		myInputCallbacks[aCallbakId].myCallback = nullptr;
	}

	uint InputManager::AddScrollCallback(std::function<void(double, double)> aCallback, GLFWwindow* aWindow /*= nullptr*/)
	{
		GLFWwindow* window = aWindow ? aWindow : myMainWindow;
		if (!window)
			return UINT_MAX;

		for (uint i = 0; i < myScrollInputCallbacks.size(); ++i)
		{
			ScrollInputCallback& callback = myScrollInputCallbacks[i];
			if (callback.myCallback == nullptr)
			{
				callback.myWindow = window;
				callback.myCallback = aCallback;
				return i;
			}
		}

		myScrollInputCallbacks.push_back({ window, aCallback });
		return (uint)myScrollInputCallbacks.size() - 1;
	}

	void InputManager::RemoveScrollCallback(uint aCallbakId)
	{
		if (aCallbakId >= myScrollInputCallbacks.size())
			return;

		myScrollInputCallbacks[aCallbakId].myCallback = nullptr;
	}

	void InputManager::KeyCallback(GLFWwindow* aWindow, int aKey, int aScanCode, int anAction, int someMods)
	{
		(void)aScanCode;
		(void)anAction;
		(void)someMods;

		for(auto callback : ourInstance->myInputCallbacks)
		{
			if(callback.myCallback != nullptr && aWindow == callback.myWindow && aKey == locRawInputToGlfwInput(callback.myInput))
			{
				callback.myCallback();
			}
		}
	}

	void InputManager::MouseCallback(GLFWwindow* aWindow, int aButton, int anAction, int someMods)
	{
		(void)anAction;
		(void)someMods;

		for(auto callback : ourInstance->myInputCallbacks)
		{
			if (callback.myCallback != nullptr && aWindow == callback.myWindow && aButton == locRawInputToGlfwInput(callback.myInput))
			{
				callback.myCallback();
			}
		}
	}

	void InputManager::ScrollCallback(GLFWwindow* aWindow, double anX, double anY)
	{
		for (auto callback : ourInstance->myScrollInputCallbacks)
		{
			if (callback.myCallback != nullptr && aWindow == callback.myWindow)
			{
				callback.myCallback(anX, anY);
			}
		}
	}
}
