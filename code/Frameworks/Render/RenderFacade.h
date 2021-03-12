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

		static void InitVulkanRenderer(GLFWwindow* aWindow);
		static void UpdateVulkanRenderer();
		static void FinalizeVulkanRenderer(GLFWwindow* aWindow);

	private:
		static BasicRenderer* ourBasicRenderer;
	};
}
