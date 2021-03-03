#include <GLFW/glfw3.h>
#include "GameWork.h"

GameWork* GameWork::ourInstance = nullptr;
Input::InputManager* GameWork::myInputManager = nullptr;

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
    Input::InputManager *myInputManager = new Input::InputManager();
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
        if(myInputManager->PollKeyInput(ourInstance->myWindow, Input::KeyW))
            std::cout << "FORWARD";
        else if(myInputManager->PollKeyInput(ourInstance->myWindow, Input::KeyA))
            std::cout << "LEFT";
        else if(myInputManager->PollKeyInput(ourInstance->myWindow, Input::KeyD))
            std::cout << "RIGHT";
        else if(myInputManager->PollKeyInput(ourInstance->myWindow, Input::KeyS))
            std::cout << "BACKWARD";

        vec2 mousePosition = myInputManager->PollMousePosition(ourInstance->myWindow);
        std::cout << " | Mouse position: " << mousePosition.x << " : " << mousePosition.y << std::endl;
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
