#pragma once

struct GLFWwindow;

namespace Render
{
	class BasicRenderer;

	class Facade
	{
	public:
		static void InitBasicRenderer(GLFWwindow* aWindow);
		static void UpdateBasicRenderer();
		static void FinalizeBasicRenderer();

	private:
		static BasicRenderer* ourBasicRenderer;
	};
}
