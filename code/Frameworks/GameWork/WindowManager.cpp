#include "WindowManager.h"

#include "Input.h"
#include "GameWork.h"

#include <GLFW/glfw3.h>

namespace GameWork
{
	WindowManager::WindowManager()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	}

	WindowManager::~WindowManager()
	{
		glfwTerminate();
	}

	GLFWwindow* WindowManager::OpenWindow(int aWidth, int aHeight, const char* aTitle)
	{
		GLFWwindow* window = glfwCreateWindow(aWidth, aHeight, aTitle, nullptr, nullptr);
		Input::InputManager::GetInstance()->AddWindow(window);
		RegisterCallbacks(window);
		return window;
	}

	void WindowManager::CloseWindow(GLFWwindow* aWindow)
	{
		UnregisterCallbacks(aWindow);
		Input::InputManager::GetInstance()->RemoveWindow(aWindow);
		glfwDestroyWindow(aWindow);
	}

	void WindowManager::RegisterCallbacks(GLFWwindow* aWindow)
	{
		glfwSetWindowSizeCallback(aWindow, OnSizeCallback);
		glfwSetFramebufferSizeCallback(aWindow, OnFramebufferSizeCallback);
		glfwSetKeyCallback(aWindow, OnKeyCallback);
		glfwSetMouseButtonCallback(aWindow, OnMouseCallback);
		glfwSetScrollCallback(aWindow, OnScrollCallback);
	}

	void WindowManager::UnregisterCallbacks(GLFWwindow* aWindow)
	{
		glfwSetWindowSizeCallback(aWindow, nullptr);
		glfwSetFramebufferSizeCallback(aWindow, nullptr);
		glfwSetKeyCallback(aWindow, nullptr);
		glfwSetMouseButtonCallback(aWindow, nullptr);
		glfwSetScrollCallback(aWindow, nullptr);
	}

	void WindowManager::OnSizeCallback(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		GameWork::OnWindowResize(aWindow, aWidth, aHeight);
	}

	void WindowManager::OnFramebufferSizeCallback(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		GameWork::OnWindowFramebufferResize(aWindow, aWidth, aHeight);
	}

	void WindowManager::OnKeyCallback(GLFWwindow* aWindow, int aKey, int aScanCode, int anAction, int someMods)
	{
		Input::InputManager::KeyCallback(aWindow, aKey, aScanCode, anAction, someMods);
	}

	void WindowManager::OnMouseCallback(GLFWwindow* aWindow, int aButton, int anAction, int someMods)
	{
		Input::InputManager::MouseCallback(aWindow, aButton, anAction, someMods);
	}

	void WindowManager::OnScrollCallback(GLFWwindow* aWindow, double anX, double anY)
	{
		Input::InputManager::ScrollCallback(aWindow, anX, anY);
	}
}
