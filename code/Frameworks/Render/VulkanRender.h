#pragma once

#include "RenderModel.h"
#include "RenderRenderer.h"
#include "VulkanImage.h"
#include "VulkanModel.h"

struct GLFWwindow;

namespace Render::Vulkan
{
	struct Device;
	class SwapChain;

	class RenderCore
	{
	public:
		static void Create();
		static void Destroy();
		static RenderCore* GetInstance() { return ourInstance; }

		void Initialize();
		void Finalize();

		void StartFrame();
		void EndFrame();

		void RegisterWindow(GLFWwindow* aWindow, RendererType aType);
		void UnregisterWindow(GLFWwindow* aWindow);
		Render::Renderer* GetRenderer(GLFWwindow* aWindow);

		Render::Model* SpawnModel(const BaseModelData& someData);
		void DespawnModel(Render::Model* aModel);

		VkInstance GetVkInstance() const { return myVkInstance; }

		Device* GetVulkanDevice() const { return myDevice; }
		VkPhysicalDevice GetPhysicalDevice() const;
		VkDevice GetDevice() const;
		VmaAllocator GetAllocator() const;
		VkQueue GetGraphicsQueue() const;
		VkCommandPool GetGraphicsCommandPool() const;

	private:
		static RenderCore* ourInstance;

		RenderCore();
		~RenderCore();

		void CreateVkInstance();
		VkInstance myVkInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT myDebugMessenger = VK_NULL_HANDLE;
		void CreateDevice();
		Device* myDevice = nullptr;

		void SetupEmptyTexture();
		Image myMissingTexture;

		std::vector<SwapChain*> mySwapChains;

		uint GetModelsLifetime() const;
		void DeleteUnusedModels(bool aDeleteNow = false);
		typedef std::pair<Model*, uint> ModelLifeTime;
		std::vector<ModelLifeTime> myModelsToDelete;
	};
}
