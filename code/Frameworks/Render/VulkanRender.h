#pragma once

#include "Renderer.h"
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

		void RegisterWindow(GLFWwindow* aWindow);
		void UnregisterWindow(GLFWwindow* aWindow);
		SwapChain* GetWindowSwapChain(GLFWwindow* aWindow);

		Renderer* CreateRenderer(RendererType aType);
		void DestroyRenderer(Renderer* aRenderer);

		VkInstance GetVkInstance() const { return myVkInstance; }

		Device* GetVulkanDevice() const { return myDevice; }
		VkPhysicalDevice GetPhysicalDevice() const;
		VkDevice GetDevice() const;
		VmaAllocator GetAllocator() const;
		VkQueue GetGraphicsQueue() const;
		VkCommandPool GetGraphicsCommandPool() const;

		uint SpawnModel(const std::string& aFilePath, const Model::RenderData& someRenderData);
		void DespawnModel(uint anIndex);

	private:
		static RenderCore* ourInstance;

		RenderCore();
		~RenderCore();

		void CreateVkInstance();
		void CreateDevice();

		void SetupEmptyTexture();

		int GetWindowSwapChainIndex(GLFWwindow* aWindow);

		VkInstance myVkInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT myDebugMessenger = VK_NULL_HANDLE;

		Device* myDevice = nullptr;

		Image myMissingTexture;

		// One SwapChain per window/surface
		std::vector<SwapChain*> mySwapChains;
	};
}
