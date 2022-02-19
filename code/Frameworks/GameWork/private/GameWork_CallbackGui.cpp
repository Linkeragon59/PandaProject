#include "GameWork_CallbackGui.h"

namespace GameWork
{
	CallbackGui::CallbackGui(GLFWwindow* aWindow, std::function<void()> aCallback)
		: Gui(aWindow)
	{
		myCallback = std::move(aCallback);
	}

	void CallbackGui::InternalUpdate()
	{
		if (myCallback)
			myCallback();
	}
}