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
		void ScrollCallback(double aX, double aY);
		uint myScrollCallbackId = 0;
		double myXScroll = 0.0;
		double myYScroll = 0.0;

		void CharacterCallback(uint aUnicodeCodePoint);
		uint myCharacterCallbackId = 0;
		std::queue<uint> myTextInput;

		Render::Handle myGui = Render::NullHandle;
		GLFWwindow* myWindow = nullptr;
	};
}
