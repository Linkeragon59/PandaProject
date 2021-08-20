#pragma once

struct GLFWwindow;

namespace GameWork
{
	class WindowManager
	{
	public:
		WindowManager();
		~WindowManager();

		GLFWwindow* OpenWindow(int aWidth, int aHeight, const char* aTitle);
		void CloseWindow(GLFWwindow* aWindow);

	private:
		void RegisterCallbacks(GLFWwindow* aWindow);
		void UnregisterCallbacks(GLFWwindow* aWindow);

		static void OnSizeCallback(GLFWwindow* aWindow, int aWidth, int aHeight);
		static void OnFramebufferSizeCallback(GLFWwindow* aWindow, int aWidth, int aHeight);
		static void OnKeyCallback(GLFWwindow* aWindow, int aKey, int aScanCode, int anAction, int someMods);
		static void OnMouseCallback(GLFWwindow* aWindow, int aButton, int anAction, int someMods);
		static void OnScrollCallback(GLFWwindow* aWindow, double anX, double anY);
	};
}
