#include <GLFW/glfw3.h>
#include "Input.h"

namespace Input
{
    namespace
    {
        int locRawInputToGlfwInput(RawInput aRawInput)
        {
            switch (aRawInput)
            {
            case RawInput::MouseLeft:
                return GLFW_MOUSE_BUTTON_LEFT;
            case RawInput::MouseRight:
                return GLFW_MOUSE_BUTTON_RIGHT;

            case RawInput::KeyA:
                return GLFW_KEY_A;
            case RawInput::KeyD:
                return GLFW_KEY_D;
            case RawInput::KeyI:
                return GLFW_KEY_I;
            case RawInput::KeyS:
                return GLFW_KEY_S;
            case RawInput::KeyW:
                return GLFW_KEY_W;

            case RawInput::KeySpace:
                return GLFW_KEY_SPACE;
            case RawInput::KeyEscape:
                return GLFW_KEY_ESCAPE;

            default:
                return GLFW_KEY_UNKNOWN;
            }
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

	InputManager::InputManager()
	{
	}

    InputManager::~InputManager()
	{
	}

    void InputManager::Create()
    {
        ourInstance = new InputManager;
    }
    
    void InputManager::Destroy()
    {
        delete ourInstance;
        ourInstance = nullptr;
    }

    RawInputState InputManager::PollRawInput(RawInput anInput, unsigned int aWindowIdx)
    {
        if (aWindowIdx >= myWindows.size())
            return RawInputState::Unknown;

        if (anInput <= RawInput::MouseRight)
            return locGlfwInputStateToRawInputState(glfwGetMouseButton(myWindows[aWindowIdx], locRawInputToGlfwInput(anInput)));
        else
            return locGlfwInputStateToRawInputState(glfwGetKey(myWindows[aWindowIdx], locRawInputToGlfwInput(anInput)));
    }

	void InputManager::PollMousePosition(double& anOutX, double& anOutY, unsigned int aWindowIdx)
	{
        if (aWindowIdx >= myWindows.size())
        {
            anOutX = 0;
            anOutY = 0;
            return;
        }

        glfwGetCursorPos(myWindows[aWindowIdx], &anOutX, &anOutY);
	}

    void InputManager::AddCallback(RawInput anInput, std::function<void()> aCallbackFunction)
	{
		InputCallback callback{anInput, aCallbackFunction};
		myInputCallbacks.push_back(callback);
	}

	void InputManager::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		(void) window;
		(void) scancode;
		(void) action;
		(void) mods;

		for(auto callback : ourInstance->myInputCallbacks)
		{
			if(key == locRawInputToGlfwInput(callback.myInput))
			{
				callback.myCallback();
			}
		}
	}

	void InputManager::MouseCallback(GLFWwindow* window, int button, int action, int mods)
	{
		(void) window;
		(void) action;
		(void) mods;

		for(auto callback : ourInstance->myInputCallbacks)
		{
			if(button == locRawInputToGlfwInput(callback.myInput))
			{
				callback.myCallback();
			}
		}
	}

	void InputManager::SetupCallback(unsigned int aWindowIdx)
	{
		glfwSetKeyCallback(myWindows[aWindowIdx], InputManager::KeyCallback);
		glfwSetMouseButtonCallback(myWindows[aWindowIdx], InputManager::MouseCallback);
	}
}
