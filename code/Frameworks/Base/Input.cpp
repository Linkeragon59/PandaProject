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
}
