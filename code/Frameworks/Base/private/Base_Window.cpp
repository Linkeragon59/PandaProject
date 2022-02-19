#include "Base_Window.h"
#include "Base_Input.h"

#include <GLFW/glfw3.h>

namespace Window
{
	const uint locWindowDefaultWidth = 1280;
	const uint locWindowDefaultHeight = 720;

	WindowManager* WindowManager::ourInstance = nullptr;

	void WindowManager::Create()
	{
		ourInstance = new WindowManager;
	}

	void WindowManager::Destroy()
	{
		SafeDelete(ourInstance);
	}

	GLFWwindow* WindowManager::OpenWindow(const char* aTitle, int aWidth /*= 0*/, int aHeight /*= 0*/)
	{
		if (aWidth <= 0)
			aWidth = locWindowDefaultWidth;
		if (aHeight <= 0)
			aHeight = locWindowDefaultHeight;

		GLFWwindow* window = glfwCreateWindow(aWidth, aHeight, aTitle, nullptr, nullptr);
		myWindows.push_back(window);
		RegisterCallbacks(window);
		return window;
	}

	void WindowManager::CloseWindow(GLFWwindow* aWindow)
	{
		UnregisterCallbacks(aWindow);
		auto it = std::find(myWindows.begin(), myWindows.end(), aWindow);
		if (it != myWindows.end())
			myWindows.erase(it);
		glfwDestroyWindow(aWindow);
	}

	uint WindowManager::AddWindowSizeCallback(SizeCallback aCallback, GLFWwindow* aWindow)
	{
		SizeCallbackEntry entry;
		entry.myWindow = aWindow;
		entry.myCallback = aCallback;
		return myWindowSizeCallbacks.Add(entry);
	}

	void WindowManager::RemoveWindowSizeCallback(uint aCallbakId)
	{
		myWindowSizeCallbacks.Remove(aCallbakId);
	}

	uint WindowManager::AddFramebufferSizeCallback(SizeCallback aCallback, GLFWwindow* aWindow)
	{
		SizeCallbackEntry entry;
		entry.myWindow = aWindow;
		entry.myCallback = aCallback;
		return myFramebufferSizeCallbacks.Add(entry);
	}

	void WindowManager::RemoveFramebufferSizeCallback(uint aCallbakId)
	{
		myFramebufferSizeCallbacks.Remove(aCallbakId);
	}

	void WindowManager::OnSizeCallback(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		for (const SizeCallbackEntry& entry : ourInstance->myWindowSizeCallbacks.myEntries)
		{
			if (entry.IsSet() && aWindow == entry.myWindow)
			{
				entry.myCallback(aWidth, aHeight);
			}
		}
	}

	void WindowManager::OnFramebufferSizeCallback(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		for (const SizeCallbackEntry& entry : ourInstance->myFramebufferSizeCallbacks.myEntries)
		{
			if (entry.IsSet() && aWindow == entry.myWindow)
			{
				entry.myCallback(aWidth, aHeight);
			}
		}
	}

	WindowManager::WindowManager()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	}

	WindowManager::~WindowManager()
	{
		glfwTerminate();
	}

	void WindowManager::RegisterCallbacks(GLFWwindow* aWindow)
	{
		glfwSetWindowSizeCallback(aWindow, WindowManager::OnSizeCallback);
		glfwSetFramebufferSizeCallback(aWindow, WindowManager::OnFramebufferSizeCallback);
		glfwSetMouseButtonCallback(aWindow, Input::InputManager::OnMouseCallback);
		glfwSetKeyCallback(aWindow, Input::InputManager::OnKeyCallback);
		glfwSetScrollCallback(aWindow, Input::InputManager::OnScrollCallback);
		glfwSetCharCallback(aWindow, Input::InputManager::OnCharacterCallback);
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
}
