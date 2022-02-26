#include "GameCore_CallbackGui.h"

namespace GameCore
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