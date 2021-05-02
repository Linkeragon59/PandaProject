#pragma once

struct GLFWwindow;

namespace Render
{
	struct Camera;
	namespace Vulkan
	{
		class Renderer;
	}

	class Facade
	{
	public:
		static void Create();
		static void Destroy();
		static Facade* GetInstance() { return ourInstance; }

		// Render Camera
		Camera* GetRenderCamera() const { return myCamera; }

		// Vulkan Renderer
		Vulkan::Renderer* GetVulkanRenderer() const { return myVulkanRenderer; }
		// TODO: We may want to support adding/removing windows dynamically
		// TODO: We may want to be able to init the Vulkan Renderer without a window
		void InitVulkanRenderer(GLFWwindow* aWindow);
		void UpdateVulkanRenderer();
		void FinalizeVulkanRenderer(GLFWwindow* aWindow);

	private:
		static Facade* ourInstance;

		Facade();
		~Facade();

		Camera* myCamera = nullptr;
		Vulkan::Renderer* myVulkanRenderer = nullptr;
	};
}
