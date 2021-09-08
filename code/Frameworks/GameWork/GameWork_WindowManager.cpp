#include "GameWork_WindowManager.h"

#include "Base_Input.h"
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
		RegisterCallbacks(window);
		return window;
	}

	void WindowManager::CloseWindow(GLFWwindow* aWindow)
	{
		UnregisterCallbacks(aWindow);
		glfwDestroyWindow(aWindow);
	}

	void WindowManager::RegisterCallbacks(GLFWwindow* aWindow)
	{
		glfwSetWindowSizeCallback(aWindow, OnSizeCallback);
		glfwSetFramebufferSizeCallback(aWindow, OnFramebufferSizeCallback);
		glfwSetMouseButtonCallback(aWindow, OnMouseCallback);
		glfwSetKeyCallback(aWindow, OnKeyCallback);
		glfwSetScrollCallback(aWindow, OnScrollCallback);
		glfwSetCharCallback(aWindow, OnCharacterCallback);
	}

	void WindowManager::UnregisterCallbacks(GLFWwindow* aWindow)
	{
		glfwSetWindowSizeCallback(aWindow, nullptr);
		glfwSetFramebufferSizeCallback(aWindow, nullptr);
		glfwSetMouseButtonCallback(aWindow, nullptr);
		glfwSetKeyCallback(aWindow, nullptr);
		glfwSetScrollCallback(aWindow, nullptr);
		glfwSetCharCallback(aWindow, nullptr);
	}

	void WindowManager::OnSizeCallback(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		GameWork::OnWindowResize(aWindow, aWidth, aHeight);
	}

	void WindowManager::OnFramebufferSizeCallback(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		GameWork::OnWindowFramebufferResize(aWindow, aWidth, aHeight);
	}

	void WindowManager::OnMouseCallback(GLFWwindow* aWindow, int aButton, int anAction, int someMods)
	{
		Input::InputManager::OnMouseCallback(aWindow, aButton, anAction, someMods);
	}

	void WindowManager::OnKeyCallback(GLFWwindow* aWindow, int aKey, int aScanCode, int anAction, int someMods)
	{
		Input::InputManager::OnKeyCallback(aWindow, aKey, aScanCode, anAction, someMods);
	}

	void WindowManager::OnScrollCallback(GLFWwindow* aWindow, double anX, double anY)
	{
		Input::InputManager::OnScrollCallback(aWindow, anX, anY);
	}

	void WindowManager::OnCharacterCallback(GLFWwindow* aWindow, uint aUnicodeCodePoint)
	{
		Input::InputManager::OnCharacterCallback(aWindow, aUnicodeCodePoint);
	}
}
