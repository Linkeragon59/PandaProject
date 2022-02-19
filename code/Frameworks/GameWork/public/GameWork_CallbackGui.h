#pragma once

#include "GameWork_Gui.h"

#include <functional>

namespace GameWork
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
