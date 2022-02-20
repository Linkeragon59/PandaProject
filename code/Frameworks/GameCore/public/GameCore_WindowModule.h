#pragma once
#include "GameCore_Module.h"
#include "GameCore_SlotVector.h"

#include <functional>

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
}

namespace GameCore
{
	class WindowModule : public Module
	{
	DECLARE_GAMECORE_MODULE(WindowModule, "Window")

	protected:
		void OnRegister() override;
		void OnUnregister() override;
		void OnUpdate(GameCore::Module::UpdateType /*aType*/) override {}

	public:
		GLFWwindow* GetMainWindow() const { return (myWindows.size() > 0) ? myWindows[0] : nullptr; }
		GLFWwindow* OpenWindow(const char* aTitle, int aWidth = 0, int aHeight = 0);
		void CloseWindow(GLFWwindow* aWindow);

		uint AddWindowSizeCallback(Window::SizeCallback aCallback, GLFWwindow* aWindow);
		void RemoveWindowSizeCallback(uint aCallbakId);

		uint AddFramebufferSizeCallback(Window::SizeCallback aCallback, GLFWwindow* aWindow);
		void RemoveFramebufferSizeCallback(uint aCallbakId);

	protected:
		static void OnSizeCallback(GLFWwindow* aWindow, int aWidth, int aHeight);
		static void OnFramebufferSizeCallback(GLFWwindow* aWindow, int aWidth, int aHeight);

	private:
		void RegisterCallbacks(GLFWwindow* aWindow);
		void UnregisterCallbacks(GLFWwindow* aWindow);

		std::vector<GLFWwindow*> myWindows;

		SlotVector<Window::SizeCallbackEntry> myWindowSizeCallbacks;
		SlotVector<Window::SizeCallbackEntry> myFramebufferSizeCallbacks;
	};
}
