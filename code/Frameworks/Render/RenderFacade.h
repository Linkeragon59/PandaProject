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
		void InitRenderer();
		void UpdateRenderer();
		void FinalizeRenderer();
		void OpenWindow(GLFWwindow* aWindow);
		void CloseWindow(GLFWwindow* aWindow);
		void SetWindowView(GLFWwindow* aWindow, const glm::mat4& aView, const glm::mat4& aProjection);

		uint SpawnModel(const std::string& aFilePath, const RenderData& aRenderData);
		void DespawnModel(uint anIndex);

	private:
		static Facade* ourInstance;

		Facade();
		~Facade();

		Vulkan::Renderer* myVulkanRenderer = nullptr;
	};
}
