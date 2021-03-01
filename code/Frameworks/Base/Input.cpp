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

    int InputManager::PollInput(GLFWwindow* aWindow, MouseInput anInput)
    {
            return glfwGetMouseButton(aWindow, anInput);
    }
}
