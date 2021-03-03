#include <GLFW/glfw3.h>
#include "Input.h"

namespace Input
{
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

    int InputManager::PollMouseInput(GLFWwindow* window, MouseInput input)
    {
            return glfwGetMouseButton(window, input);
    }

    vec2 InputManager::PollMousePosition(GLFWwindow* aWindow)
    {
        double xPosition, yPosition;
        glfwGetCursorPos(aWindow, &xPosition, &yPosition);
        vec2 position;
        position.x = xPosition;
        position.y = yPosition;
        return position;
    }

    int InputManager::PollKeyInput(GLFWwindow* window, KeyInput input)
    {
            return glfwGetKey(window, input);
    }
}
