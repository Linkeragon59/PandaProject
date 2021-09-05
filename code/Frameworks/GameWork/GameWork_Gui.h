#pragma once

struct GLFWwindow;
struct ImGuiContext;

namespace Render
{
	class Gui;
}

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
		GLFWwindow* myWindow = nullptr;
		Render::Gui* myRenderGui = nullptr;
	};
}
