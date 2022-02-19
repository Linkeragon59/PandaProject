#pragma once

#include "GameCore_Gui.h"

#include <functional>

namespace GameCore
{
	class CallbackGui : public Gui
	{
	public:
		CallbackGui(GLFWwindow* aWindow, std::function<void()> aCallback);

	protected:
		void InternalUpdate() override;

	private:
		std::function<void()> myCallback;
	};
}
