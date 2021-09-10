#pragma once

#include <functional>

#include "Base_SlotVector.h"

struct GLFWwindow;

namespace Window
{
	typedef std::function<void(int, int)> SizeCallback;
	struct SizeCallbackEntry
	{
		void Clear() { myWindow = nullptr; myCallback = nullptr; }
		bool IsSet() const { return myCallback != nullptr; }
		GLFWwindow* myWindow = nullptr;
		SizeCallback myCallback = nullptr;
	};

	class WindowManager
	{
	public:
		static void Create();
		static void Destroy();
		static WindowManager* GetInstance() { return ourInstance; }

		GLFWwindow* GetMainWindow() const { return (myWindows.size() > 0) ? myWindows[0] : nullptr; }
		GLFWwindow* OpenWindow(const char* aTitle, int aWidth = 0, int aHeight = 0);
		void CloseWindow(GLFWwindow* aWindow);

		uint AddWindowSizeCallback(SizeCallback aCallback, GLFWwindow* aWindow);
		void RemoveWindowSizeCallback(uint aCallbakId);

		uint AddFramebufferSizeCallback(SizeCallback aCallback, GLFWwindow* aWindow);
		void RemoveFramebufferSizeCallback(uint aCallbakId);

	protected:
		static void OnSizeCallback(GLFWwindow* aWindow, int aWidth, int aHeight);
		static void OnFramebufferSizeCallback(GLFWwindow* aWindow, int aWidth, int aHeight);

	private:
		static WindowManager* ourInstance;
		WindowManager();
		~WindowManager();

		void RegisterCallbacks(GLFWwindow* aWindow);
		void UnregisterCallbacks(GLFWwindow* aWindow);

		std::vector<GLFWwindow*> myWindows;

		SlotVector<SizeCallbackEntry> myWindowSizeCallbacks;
		SlotVector<SizeCallbackEntry> myFramebufferSizeCallbacks;
	};
}
