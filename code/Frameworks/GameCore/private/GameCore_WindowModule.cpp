#include "GameCore_WindowModule.h"
#include "GameCore_InputModule.h"

#include <GLFW/glfw3.h>

namespace GameCore
{
	const uint locWindowDefaultWidth = 1280;
	const uint locWindowDefaultHeight = 720;

	DEFINE_GAMECORE_MODULE(WindowModule);

	void WindowModule::OnRegister()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	}

	void WindowModule::OnUnregister()
	{
		glfwTerminate();
	}

	GLFWwindow* WindowModule::OpenWindow(const char* aTitle, int aWidth /*= 0*/, int aHeight /*= 0*/)
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

	void WindowModule::CloseWindow(GLFWwindow* aWindow)
	{
		UnregisterCallbacks(aWindow);
		auto it = std::find(myWindows.begin(), myWindows.end(), aWindow);
		if (it != myWindows.end())
			myWindows.erase(it);
		glfwDestroyWindow(aWindow);
	}

	uint WindowModule::AddWindowSizeCallback(Window::SizeCallback aCallback, GLFWwindow* aWindow)
	{
		Window::SizeCallbackEntry entry;
		entry.myWindow = aWindow;
		entry.myCallback = aCallback;
		return myWindowSizeCallbacks.Add(entry);
	}

	void WindowModule::RemoveWindowSizeCallback(uint aCallbakId)
	{
		myWindowSizeCallbacks.Remove(aCallbakId);
	}

	uint WindowModule::AddFramebufferSizeCallback(Window::SizeCallback aCallback, GLFWwindow* aWindow)
	{
		Window::SizeCallbackEntry entry;
		entry.myWindow = aWindow;
		entry.myCallback = aCallback;
		return myFramebufferSizeCallbacks.Add(entry);
	}

	void WindowModule::RemoveFramebufferSizeCallback(uint aCallbakId)
	{
		myFramebufferSizeCallbacks.Remove(aCallbakId);
	}

	void WindowModule::OnSizeCallback(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		for (const Window::SizeCallbackEntry& entry : ourInstance->myWindowSizeCallbacks.myEntries)
		{
			if (entry.IsSet() && aWindow == entry.myWindow)
			{
				entry.myCallback(aWidth, aHeight);
			}
		}
	}

	void WindowModule::OnFramebufferSizeCallback(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		for (const Window::SizeCallbackEntry& entry : ourInstance->myFramebufferSizeCallbacks.myEntries)
		{
			if (entry.IsSet() && aWindow == entry.myWindow)
			{
				entry.myCallback(aWidth, aHeight);
			}
		}
	}

	void WindowModule::RegisterCallbacks(GLFWwindow* aWindow)
	{
		glfwSetWindowSizeCallback(aWindow, WindowModule::OnSizeCallback);
		glfwSetFramebufferSizeCallback(aWindow, WindowModule::OnFramebufferSizeCallback);
		glfwSetMouseButtonCallback(aWindow, InputModule::OnMouseCallback);
		glfwSetKeyCallback(aWindow, InputModule::OnKeyCallback);
		glfwSetScrollCallback(aWindow, InputModule::OnScrollCallback);
		glfwSetCharCallback(aWindow, InputModule::OnCharacterCallback);
	}

	void WindowModule::UnregisterCallbacks(GLFWwindow* aWindow)
	{
		glfwSetWindowSizeCallback(aWindow, nullptr);
		glfwSetFramebufferSizeCallback(aWindow, nullptr);
		glfwSetMouseButtonCallback(aWindow, nullptr);
		glfwSetKeyCallback(aWindow, nullptr);
		glfwSetScrollCallback(aWindow, nullptr);
		glfwSetCharCallback(aWindow, nullptr);
	}
}
