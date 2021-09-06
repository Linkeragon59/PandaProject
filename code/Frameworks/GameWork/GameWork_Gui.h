#pragma once

#include "Render_Handle.h"

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
		Render::Handle myGui = Render::NullHandle;
		GLFWwindow* myWindow = nullptr;
	};
}
