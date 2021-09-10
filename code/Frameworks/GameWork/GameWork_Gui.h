#pragma once

#include "Render_Handle.h"

#include <queue>

struct GLFWwindow;
struct ImGuiContext;

namespace GameWork
{
	class Gui
	{
	public:
		Gui(GLFWwindow* aWindow);
		virtual ~Gui();

		void Update();
		void Draw();

	protected:
		virtual void InternalUpdate() = 0;
		ImGuiContext* myGuiContext = nullptr;

	private:
		uint myScrollCallbackId = UINT_MAX;
		double myXScroll = 0.0;
		double myYScroll = 0.0;

		uint myCharacterCallbackId = UINT_MAX;
		std::queue<uint> myTextInput;

		Render::Handle myGui = Render::NullHandle;
		GLFWwindow* myWindow = nullptr;
	};
}
