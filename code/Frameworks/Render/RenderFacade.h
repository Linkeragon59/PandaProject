#pragma once

struct GLFWwindow;

#include "RenderData.h"

namespace Render
{
	namespace Vulkan
	{
		class Renderer;
	}
	class Camera;
	class Model;

	class Facade
	{
	public:
		static void Create();
		static void Destroy();
		static Facade* GetInstance() { return ourInstance; }

		// Vulkan Renderer
		Vulkan::Renderer* GetRenderer() const { return myVulkanRenderer; }
		// TODO: We may want to support adding/removing windows dynamically
		// TODO: We may want to be able to init the Vulkan Renderer without a window
		void InitRenderer(GLFWwindow* aWindow);
		void UpdateRenderer(const glm::mat4& aView, const glm::mat4& aProjection);
		void FinalizeRenderer(GLFWwindow* aWindow);

		uint SpawnModel(const std::string& aFilePath, const RenderData& aRenderData);
		void DespawnModel(uint anIndex);

	private:
		static Facade* ourInstance;

		Facade();
		~Facade();

		Vulkan::Renderer* myVulkanRenderer = nullptr;
	};
}
