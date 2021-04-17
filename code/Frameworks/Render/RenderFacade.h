#pragma once

struct GLFWwindow;

namespace Render
{
	class Facade
	{
	public:
		static void InitVulkanRenderer(GLFWwindow* aWindow);
		static void UpdateVulkanRenderer();
		static void FinalizeVulkanRenderer(GLFWwindow* aWindow);
	};
}
