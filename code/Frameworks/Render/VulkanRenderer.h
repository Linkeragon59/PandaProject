#pragma once

#include "VulkanImage.h"

struct GLFWwindow;

namespace Render
{
namespace glTF
{
	class Model;
}
namespace Vulkan
{
	struct Device;
	class SwapChain;

	class Renderer
	{
	public:
		// Shortcut using the Facade singleton
		static Renderer* GetInstance();

		Renderer();
		~Renderer();

		void Init();
		void Finalize();

		void OnWindowOpened(GLFWwindow* aWindow);
		void OnWindowClosed(GLFWwindow* aWindow);

		void Update();

		VkInstance GetVkInstance() const { return myVkInstance; }

		Device* GetVulkanDevice() const { return myDevice; }
		VkPhysicalDevice GetPhysicalDevice() const;
		VkDevice GetDevice() const;
		VmaAllocator GetAllocator() const;
		VkQueue GetGraphicsQueue() const;
		VkCommandPool GetGraphicsCommandPool() const;

	private:
		void CreateVkInstance();
		void CreateDevice();

		void SetupEmptyTexture();

		VkInstance myVkInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT myDebugMessenger = VK_NULL_HANDLE;

		Device* myDevice = nullptr;

		// One SwapChain per window/surface
		std::vector<SwapChain*> mySwapChains;

		Image myMissingTexture;
	};
}
}
