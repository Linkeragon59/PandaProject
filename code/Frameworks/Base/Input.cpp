#include <GLFW/glfw3.h>
#include "Input.h"

namespace Base
{
    namespace
	{
		const uint32_t locWindowWidth = 800;
		const uint32_t locWindowHeight = 600;
    }

    InputManager* InputManager::ourInstance = nullptr;

	InputManager::InputManager()
	{
		InitWindow();
	}

    InputManager::~InputManager()
	{
		Cleanup();
	}

    void InputManager::InitWindow()
	{
		glfwInit();
		myWindow = glfwCreateWindow(locWindowWidth, locWindowHeight, "GLFW input test", nullptr, nullptr);
	}

    void InputManager::Cleanup()
    {
        glfwDestroyWindow(myWindow);
        glfwTerminate();
    }

    void InputManager::PollInput(MouseInput anInput)
    {
        bool clicked = false;

        while (!glfwWindowShouldClose(myWindow))
		{
            glfwPollEvents();
            int isButtonClicked = glfwGetMouseButton(myWindow, anInput);
            if(isButtonClicked == GLFW_PRESS)
                clicked = true;
        }

        if(clicked)
            std::cout << "Button clicked" << std::endl;
        else
            std::cout << "Button not clicked" << std::endl;
    }
}
