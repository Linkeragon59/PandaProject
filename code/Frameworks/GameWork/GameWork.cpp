#include <GLFW/glfw3.h>
#include "GameWork.h"

GameWork* GameWork::ourInstance = nullptr;
Base::InputManager* GameWork::myInputManager = nullptr;

namespace
{
	const uint32_t locWindowWidth = 800;
	const uint32_t locWindowHeight = 600;
}

GameWork::GameWork()
{
}

GameWork::~GameWork()
{
}

void GameWork::Create()
{
    ourInstance = new GameWork;
    Base::InputManager *myInputManager = new Base::InputManager();
	myInputManager->Create();
    ourInstance->InitWindow();
}

void GameWork::Destroy()
{
    delete ourInstance;
    ourInstance = nullptr;
    myInputManager->Destroy();
}

void GameWork::Run()
{
    while (!glfwWindowShouldClose(ourInstance->myWindow))
	{
        glfwPollEvents();
        if(myInputManager->PollMouseInput(ourInstance->myWindow, Base::Button1))
            std::cout << "Button 1 clicked";
        if(myInputManager->PollMouseInput(ourInstance->myWindow, Base::Button2))
            std::cout << "Button 2 clicked";

        double xMousePosition = myInputManager->PollMousePosition(ourInstance->myWindow);
        std::cout << " | Mouse position: " << xMousePosition << std::endl;
    }
}

void GameWork::InitWindow()
{
	glfwInit();
	ourInstance->myWindow = glfwCreateWindow(locWindowWidth, locWindowHeight, "Panda Project v0.1", nullptr, nullptr);
}

void GameWork::Cleanup()
{
    glfwDestroyWindow(myWindow);
    glfwTerminate();
}
