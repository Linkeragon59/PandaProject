#include <GLFW/glfw3.h>
#include "Input.h"

namespace Base
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

    int InputManager::PollMouseInput(GLFWwindow* aWindow, MouseInput anInput)
    {
            return glfwGetMouseButton(aWindow, anInput);
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
}