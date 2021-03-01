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

    void InputManager::PollInput(GLFWwindow* aWindow, MouseInput anInput)
    {
        bool clicked = false;

        while (!glfwWindowShouldClose(aWindow))
		{
            glfwPollEvents();
            int isButtonClicked = glfwGetMouseButton(aWindow, anInput);
            if(isButtonClicked == GLFW_PRESS)
                clicked = true;
        }

        if(clicked)
            std::cout << "Button clicked" << std::endl;
        else
            std::cout << "Button not clicked" << std::endl;
    }
}
